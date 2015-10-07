#!/usr/bin/env python
# -*- coding: utf-8 -*-
import pybel
import openbabel
import json
import sys
import os

def bondExtractor(mol):
	#print mol.data['EMOL_VERSION_ID'] # for emolecules database
	#bond = [(obbond.GetBeginAtomIdx(), obbond.GetEndAtomIdx()) for obbond in openbabel.OBMolBondIter(mol.OBMol)]

        #print mol.data['NSC'] # for nsc database

        #print mol.data['PUBCHEM_COMPOUND_CID'] # for Protein Bank database

	return {"id": mol.data['NSC'], 
			"bond": {
				"aid1": [obbond.GetBeginAtomIdx() for obbond in openbabel.OBMolBondIter(mol.OBMol)],
				"aid2": [obbond.GetEndAtomIdx() for obbond in openbabel.OBMolBondIter(mol.OBMol)],
				}}

def singlesdfprocess(fname):
	cnt = 0
	foutput = open(fname + '.txt', 'w')
	for mol in pybel.readfile("sdf", fname):
            #print mol.data
	    cnt += 1
	    foutput.write(json.dumps(bondExtractor(mol)))
	    foutput.write('\n')
	foutput.close()
	os.system("wc  -l {0}".format(fname+'.txt'))

'''
cnt = 0
flist = os.listdir(sys.argv[1])
#cnt = flist.index("Compound_052600001_052625000.sdf.gz")

for idx in range(cnt, len(flist)):
	fname = os.path.join(sys.argv[1], flist[idx])
	if os.path.isfile(fname) and fname.endswith('.gz'):
		singlesdfprocess(fname)
	cnt += 1
	sys.stderr.write("{%.2f}\n" % (cnt*100.0/7338))

#singlesdfprocess("data/RCSB/compound/SDF/Compound_085225001_085250000.sdf.gz")

'''

singlesdfprocess(sys.argv[1])
