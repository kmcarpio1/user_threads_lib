import os
import sys

# Creator
def NewConfig(is_test, params, unit_time, labels):
    return {
        'is_test': is_test,
        'params': params,
        'unit_time': unit_time,
        'constlabel' : labels[0] if isinstance(params[0], list) else None,
        'paramlabel' : labels[1] if isinstance(params[0], list) else (labels[0] if params[0] != None else ("Instance de test")),
    }

# Configuration for all tests
all_config = {
    '21-create-many': NewConfig(True, [100, 500, 1000, 2000, 5000, 10000], 'us', ["Nombre de threads"]),
    '22-create-many-recursive': NewConfig(True, [100, 500, 1000, 2000, 5000, 10000], 'us', ["Nombre de threads"]),
    '23-create-many-once': NewConfig(True, [100, 500, 1000, 2000, 5000, 10000], 'us', ["Nombre de threads"]), 
    '31-switch-many': NewConfig(True, [[20, 100], [20, 500], [20, 1000], [20, 2000], [20, 5000], [20, 10000], [20, 20000], [20, 50000]], 'us', ["Nombre de threads", "Nombre de yields"]),
    '32-switch-many-join': NewConfig(True, [[20, 100], [20, 500], [20, 1000], [20, 2000], [20, 5000], [20, 10000], [20, 20000], [20, 50000]], 'us', ["Nombre de threads", "Nombre de yields"]), 
    '33-switch-many-cascade': NewConfig(True, [[20, 100], [20, 500], [20, 1000], [20, 2000], [20, 5000], [20, 10000], [20, 20000], [20, 50000]], 'us', ["Nombre de threads", "Nombre de yields"]), 
    '51-fibonacci': NewConfig(True, [1, 5, 10, 15, 20, 25], 's', ["Paramètre de fibo"]), 
    '61-mutex': NewConfig(True, [5, 10, 20, 50, 100, 200, 500], 'us', ["Nombre de threads"]),
    '62-mutex': NewConfig(True, [5, 10, 20, 50, 100, 200, 500], 'us', ["Nombre de threads"]),
    '63-mutex-equity': NewConfig(True, [None], 'us', [None]),
    '64-mutex-join': NewConfig(True, [None], 'us', [None]), 
    '71-preemption': NewConfig(False, [1, 5, 10, 15, 20, 25], 'us', ["Nombre de threads"]), 
    '81-deadlock': NewConfig(False, [None], '', [None]),   
    '91-stack-overflow': NewConfig(False, [None], '', [None]),
    '92-signal': NewConfig(False, [None], '', [None]),
}

# Configuration mirrored for pthread tests
updated_config = {}
for key, value in all_config.items():
    old_name = key
    new_name = old_name + '-pthread'
    updated_config[new_name] = value
all_config.update(updated_config)


# Validator
def isValidTestName(test_name): 
    return test_name in list(all_config.keys())

# Returns number of params of config (0, 1 or 2)
def nbOfParams(config):
    if isinstance(config['params'][0], list):
        return 2
    elif config['params'][0] == None :
        return 0
    return 1
        
# Return a dico with the configuration of the test
def getConfig(test_path):
    test_name = os.path.basename(test_path) 

    if not isValidTestName(test_name):
        print("error: Name test invalid")
        print("test_name:", test_name)
        sys.exit(1)

    return {
        'test_path': test_path,
        'test_name': test_name,
        'is_test':  all_config[test_name]['is_test'],
        'params': all_config[test_name]['params'],
        'unit_time': all_config[test_name]['unit_time'],
        'constlabel' : all_config[test_name]['constlabel'],
        'paramlabel' : all_config[test_name]['paramlabel']
    } 