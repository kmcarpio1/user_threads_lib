import subprocess
import matplotlib.pyplot as plt
import numpy as np
import time
import sys
import os
import csv

from config import getConfig, nbOfParams
from utils import *

# Constants
top0 = [['runTime']]
top = [['param', 'runTime']]
top2 = [['const', 'param', 'runTime']]

# If a row has param in a row, don't run the test (already done)
def not_already_ran(config_test, param):
    name_file_data = getNameFileData(config_test)
    if os.path.exists(name_file_data):
        with open(name_file_data, newline='') as fichier_csv:
            reader = csv.reader(fichier_csv)
            next(reader)
            for row in reader:
                if row[0] == str(param) or row[1] == str(param) :
                    return False
    return True

def initDataFiles(config_test):

    # Init data file
    name_file_data_pthread = getNameFileData(config_test)
    
    fichier_csv = open(name_file_data_pthread, 'w', newline='')
    writer = csv.writer(fichier_csv)
    
    if nbOfParams(config_test) == 0:
        writer.writerows(top0)
    elif nbOfParams(config_test) == 1:
        writer.writerows(top)
    elif nbOfParams(config_test) == 2:
        writer.writerows(top2)

    fichier_csv.close()


# Functions
def run_test(config_test, param, limit_runtime=False):

    data = []
    if isinstance(param, list) :
        args = [config_test['test_path'], *[str(p) for p in param]]
    else :
        args = [config_test['test_path'], str(param)]

    if limit_runtime:
        count_test = getCountTest(args, config_test)
    else :
        count_test = 20
        
    for i in range(count_test):
        result = subprocess.run(args, capture_output=True, text=True)
        runTime = getRunTime(result.stdout, config_test['unit_time'])
        print(config_test['test_name'], str(param), ":", i + 1, "/", count_test, end='\r')
        
        if (runTime != None):
            if nbOfParams(config_test) == 0:
                data.append([runTime])
            elif nbOfParams(config_test) == 1:
                data.append([param, runTime])
            elif nbOfParams(config_test) == 2:
                data.append(param + [runTime])


    # Csv file connection
    name_file_data = getNameFileData(config_test)

    with open(name_file_data, 'a', newline='') as fichier_csv:
        writer = csv.writer(fichier_csv)
        writer.writerows(data)


# Main
def main():
    test_path = sys.argv[1]
    config_test = getConfig(test_path)
    
    if not config_test['is_test']:
        print(config_test['test_name'], "is not tested")
        return

    initDataFiles(config_test)

    for param in config_test['params']:
        
        # # pthread test (Don't run if already done with the specified param)
        # if (not_already_ran(config_test, param)):
        #     run_test(config_test, param)
        #     print("")
        # else:
        #     print(config_test['test_name'], str(param), ": tests already done")

        run_test(config_test, param)
        print("")

if __name__ == "__main__":
    main()
