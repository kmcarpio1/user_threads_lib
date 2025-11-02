import subprocess
import matplotlib.pyplot as plt
import numpy as np
import time
import sys
import os
import csv
from collections import defaultdict

from config import getConfig, nbOfParams
from utils import *


def plot_csvs(config_test):
    
    # Special case for constant tests
    if nbOfParams(config_test) == 0:
        # Initialize lists to store all runtimes for each instance
        user_thread_data = list()
        pthread_data = list()

        # Extract data from CSV files and calculate all runtimes
        files = [getNameFileData(config_test), getNameFileDataAsPthread(config_test)]
        results = [user_thread_data, pthread_data]
        for i in range(2) :
            with open(files[i], 'r') as file:
                reader = csv.DictReader(file)
                for row in reader:
                    run_time = float(row['runTime'])
                    data_struct = results[i]
                    data_struct.append(run_time)

        if nbOfParams(config_test) == 0:
            plt.plot(range(len(user_thread_data)), user_thread_data, marker='o', linestyle='-', label="User threads")
            plt.plot(range(len(user_thread_data)), pthread_data, marker='o', linestyle='-', label="pthreads")
        
        plt.ylabel('Temps d\'exécution')
        plt.xlabel(config_test['paramlabel'])
        
    else :

        # Initialize dictionaries to store mean runtimes for each input value
        user_thread_data = defaultdict(list)
        pthread_data = defaultdict(list)

        # Extract data from CSV files and calculate mean runtimes
        files = [getNameFileData(config_test), getNameFileDataAsPthread(config_test)]
        results = [user_thread_data, pthread_data]
        for i in range(2) :
            with open(files[i], 'r') as file:
                reader = csv.DictReader(file)
                for row in reader:
                    if (nbOfParams(config_test) == 2) :
                        const = float(row['const'])

                    param = float(row['param'])
                    run_time = float(row['runTime'])
                    data_struct = results[i]
                    data_struct[param].append(run_time)

        # Labeling depending on parameters number
        if (nbOfParams(config_test) == 1):
            plt.ylabel('Temps d\'exécution moyen')
            plt.xlabel(config_test['paramlabel'])
        elif nbOfParams(config_test) ==  2:
            plt.ylabel('Temps d\'exécution moyen')
            plt.xlabel(config_test['paramlabel'] + f" ({config_test['constlabel']}={const})")

        
        # Calculate runtimes
        user_thread_means = {param: sum(times) / len(times) for param, times in user_thread_data.items()}
        pthread_means = {param: sum(times) / len(times) for param, times in pthread_data.items()}

        # Plot mean runtimes
        plt.plot(list(user_thread_means.keys()), list(user_thread_means.values()), marker='o', linestyle='-', label="User threads")
        plt.plot(list(pthread_means.keys()), list(pthread_means.values()), marker='o', linestyle='-', label="pthreads")
        
    plt.title(f"Comparaison des performances sur le test : {config_test['test_name']}")
    plt.grid(True)
    plt.legend()
    
    # Create directory if it does not exist
    if not os.path.exists(DIR_RESULTS):
        os.makedirs(DIR_RESULTS)

    # Save plot as image file
    plt.savefig(os.path.join(DIR_RESULTS, config_test['test_name']+'.png'))


# Main
def main():
    test_path = sys.argv[1]
    config_test = getConfig(test_path)
    
    if not config_test['is_test']:
        print(config_test['test_name'], "is not tested")
        return

    plot_csvs(config_test)

if __name__ == "__main__":
    main()
