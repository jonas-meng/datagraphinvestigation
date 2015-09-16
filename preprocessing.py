#!/usr/bin/env python
# -*- coding: utf-8 -*-
import pybel
import openbabel
import json
import sys
import os

def bondExtractor(mol):
	#print mol.data['EMOL_VERSION_ID']
	#bond = [(obbond.GetBeginAtomIdx(), obbond.GetEndAtomIdx()) for obbond in openbabel.OBMolBondIter(mol.OBMol)]

	return {"id": mol.data['EMOL_VERSION_ID'], 
			"bond": {
				"aid1": [obbond.GetBeginAtomIdx() for obbond in openbabel.OBMolBondIter(mol.OBMol)],
				"aid2": [obbond.GetEndAtomIdx() for obbond in openbabel.OBMolBondIter(mol.OBMol)],
				}}

def singlesdfprocess(fname):
    cnt = 0
    sys.stderr.write("{0} start.\n".format(fname))
    with open(fname + '.txt', 'a') as output:
        for mol in pybel.readfile("sdf", fname):
            cnt += 1
            output.write(json.dumps(bondExtractor(mol)))
            output.write('\n')
            if cnt % 1000000 == 0:
                sys.stderr.write("{0}\n".format(cnt))
    sys.stderr.write("{0} finished.\n".format(fname))

for f in os.listdir(sys.argv[1]):
    fname = os.path.join(sys.argv[1], f)
    if os.path.isfile(fname) and fname.endswith('.gz'):
        singlesdfprocess(fname)
        
