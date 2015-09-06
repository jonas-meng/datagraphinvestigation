#!/usr/bin/env python
# -*- coding: utf-8 -*-
import pybel
import openbabel
import json
import pprint
import sys

def bondExtractor(mol):
	#print mol.data['EMOL_VERSION_ID']
	#bond = [(obbond.GetBeginAtomIdx(), obbond.GetEndAtomIdx()) for obbond in openbabel.OBMolBondIter(mol.OBMol)]

	return {"id": mol.data['EMOL_VERSION_ID'], 
			"bond": {
				"aid1": [obbond.GetBeginAtomIdx() for obbond in openbabel.OBMolBondIter(mol.OBMol)],
				"aid2": [obbond.GetEndAtomIdx() for obbond in openbabel.OBMolBondIter(mol.OBMol)],
				}}
cnt = 0
for mol in pybel.readfile("sdf", "data/emolecules.sdf"):
    print json.dumps(bondExtractor(mol))
    cnt += 1
    if cnt % 100000 == 0:
        sys.stderr.write("{:.2f}\n".format(cnt / float(7775740) * 100))
sys.stderr.write("Finished.\n")
