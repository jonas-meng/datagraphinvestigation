#!/usr/bin/env python
# -*- coding: utf-8 -*-
import json
import csv
import sys
from sets import Set

with open(sys.argv[2], 'w') as output:
    writer = csv.writer(output)
    writer.writerow(['id', 'edge', 'node'])
    cnt = 0
    with open(sys.argv[1], 'r') as data:
        for line in data:
            cnt += 1
            if cnt % 100000 == 0:
                sys.stderr.write("{:.2f}\n".format(cnt / float(7775740) * 100))
            item = json.loads(line)
            writer.writerow([item["id"], len(item["bond"]["aid1"]), len(Set(item["bond"]["aid1"]+item["bond"]["aid2"]))])
