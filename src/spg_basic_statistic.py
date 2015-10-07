#!/usr/bin/env python
# -*- coding: utf-8 -*-
import json
import csv
import sys
import os
from sets import Set

with open(sys.argv[2], 'w') as output:
    writer = csv.writer(output)
    writer.writerow(['id', 'edge', 'node'])
    fcnt = 0
    flist = os.listdir(sys.argv[1])
    for idx in range(0, len(flist)):
        fname = os.path.join(sys.argv[1], flist[idx])
        if os.path.isfile(fname) and fname.endswith('.txt'):
            fcnt = fcnt + 1
            print "%s: %.2f%%" % (fname, fcnt*100.0/3669)
            data = open(fname, 'r')
            for line in data:
                item = json.loads(line)
                writer.writerow([item["id"], len(item["bond"]["aid1"]), len(Set(item["bond"]["aid1"]+item["bond"]["aid2"]))])
            data.close()
