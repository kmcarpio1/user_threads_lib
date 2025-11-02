import subprocess



#Constants

DIR_DATA = './tst/graphs/data'
DIR_RESULTS = './tst/graphs/results'

MAX_RUNTIME_US = 10000

convert_time = {
    's': 10**6,
    'us': 1
}


# Utils functions

def getRunTime(outputs, unit_time):
    l = outputs.replace('\n', ' ').split(' ')
    if (len(l) == 1 and l[0] == '') :
        return None
    i = l.index(unit_time)
    return int(float(l[i - 1]) * convert_time[unit_time])

# Computes reasonable amount of tests necessary
def getCountTest(args, config_test):
    result = subprocess.run(args, capture_output=True, text=True)
    runTime = int(getRunTime(result.stdout, config_test['unit_time']))
    return int(MAX_RUNTIME_US/runTime) + 1

def getNameFileData(config_test):
    return DIR_DATA + '/' + config_test['test_name'] + '.csv'

def getNameFileDataAsPthread(config_test):
    return DIR_DATA + '/' + config_test['test_name'] + '-pthread.csv'