import os
import json

# this is to generate a test case of a extremerly large json file.
# the test case includes:
#   - long key string
#   - long value string
#   - deep nested attay/json
#   - big .json file


def gen_long_str(str_key, num_repeat):
    
    tmp = [str_key] * num_repeat
    return ';'.join(tmp)

json_data = {}

# generate a long key string
str_key = gen_long_str('long key', 1024)
str_value = gen_long_str('long value', 1024)
json_data[str_key] = str_value

# generate a deep nested array
json_data['deep_nested_array'] = []
_tmp = json_data['deep_nested_array']
for i in range(50):
    _tmp.append(i)
    _tmp.append(str_key)
    _tmp.append(str_value)
    _tmp.append({str_key: str_value})
    _tmp.append([str_key, str_value, {str_key: str_value}])
    _tmp.append([])
    _tmp = _tmp[-1]
    
# generate a deep nested json
json_data['deep_nested_json'] = {}
_tmp = json_data['deep_nested_json']
for i in range(50):
    _tmp[str_key] = str_value
    _tmp[str_value] = str_key
    _tmp[str_key + str_value] = str_value + str_key
    _tmp[str_value + str_key] = str_key + str_value
    _tmp["deeper"] = {}
    _tmp = _tmp["deeper"]
    
# make the test json file big by dumping the json data multiple times
for i in range(8192):
    _key = f"repeat_key_{i}"
    _value = f"repeat_value_{i}"
    
    _key = gen_long_str(_key, 128)
    _value = gen_long_str(_value, 128)
    json_data[_key] = str(i)
    
with open('big.json', 'w') as f:
    json.dump(json_data, f)
    
    
# ----------------------------
# generate nested-obj.json
# ----------------------------

json_data = {}
_tmp = json_data
for i in range(100):
    _key = f"k{i}"
    _tmp[_key] = {}
    _tmp = _tmp[_key]
    
with open('nested-obj.json', 'w') as f:
    json.dump(json_data, f)
    
# ----------------------------
# generate nested-array.json
# ----------------------------

json_data = ["root"]
_tmp = json_data
for i in range(100):
    _tmp.append([])
    _tmp = _tmp[-1]
    
with open('nested-array.json', 'w') as f:
    json.dump(json_data, f)
    
    
# ----------------------------
# generate index.json
# ----------------------------
import hashlib
import random

# set random seed
random.seed(0)

json_data = {}
for i in range(1024):
    _key = f"k{i}"
    _value = f"v{i}"
    
    _key = hashlib.sha256(_key.encode()).hexdigest()
    _value = hashlib.sha256(_value.encode()).hexdigest()
    
    json_data[_key] = _value

with open('index1.json', 'w') as f:
    json.dump(json_data, f)
    
json_data = {}
for i in range(1024):
    # select random char from a-z, repeat it random times
    random_prefix = ''.join([chr(random.randint(97, 122)) for _ in range(random.randint(1, 10))])
    
    _key = f"{random_prefix}k{i}"
    _value = f"{random_prefix}v{i}"
    
    json_data[_key] = _value
    
with open('index2.json', 'w') as f:
    json.dump(json_data, f)