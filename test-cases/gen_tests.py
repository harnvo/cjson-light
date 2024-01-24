import os
import json

# this is to generate test cases of invalid json files.

json_data = {}

# -----------
# error json: not starting with {/[
# -----------

with open('error_not_starting_with_brace.json', 'w') as f:
    f.write('not starting with brace}')
    
with open('error_not_starting_with_bracket.json', 'w') as f:
    f.write('not starting with bracket]')
    
# -----------
# error json: unmatched {/[
# -----------

json_data["key1"] = "value1"

with open('error_unmatched_brace1.json', 'w') as f:
    f.write('{')
    json.dump(json_data, f)
    
    
with open('error_unmatched_bracket1.json', 'w') as f:
    f.write('[')
    json.dump(json_data, f)
    
with open('error_unmatched_brace2.json', 'w') as f:
    f.write('{')
    json.dump(json_data, f)
    f.write(']')

with open('error_unmatched_bracket2.json', 'w') as f:
    f.write('[')
    json.dump(json_data, f)
    f.write('}')
    
# -----------
# error json: too deep
# -----------

json_data = {}
_tmp = json_data
for i in range(512):
    _tmp["k"] = {}
    _tmp = _tmp["k"]
    
with open('error_too_deep1.json', 'w') as f:
    json.dump(json_data, f)
    
json_data = []
_tmp = json_data
for i in range(512):
    _tmp.append([])
    _tmp = _tmp[-1]
    
with open('error_too_deep2.json', 'w') as f:
    json.dump(json_data, f)
    
# -----------
# error json: invalid keys
# -----------

with open('error_invalid_key1.json', 'w') as f:
    f.write('{1: "value"}')

with open('error_invalid_key2.json', 'w') as f:
    f.write('{true: "value"}')
    
with open('error_invalid_key3.json', 'w') as f:
    f.write('{null: "value"}')

with open('error_invalid_key4.json', 'w') as f:
    f.write('{ops: "value"}')
    
# -----------
# error json: invalid values
# -----------

with open('error_invalid_value1.json', 'w') as f:
    f.write('{"key": 1.2.3}')
    
with open('error_invalid_value2.json', 'w') as f:
    f.write('{"key": trrue}')
    
with open('error_invalid_value3.json', 'w') as f:
    f.write('{"key": nil}')

with open('error_invalid_value4.json', 'w') as f:
    f.write('{"key": \"unterminated string}')
    
with open('error_invalid_value5.json', 'w') as f:
    f.write('{"key": \"unterminated string\\\"}')


# below generates some extra test cases for json-c

# -----------
# test json: long key
# -----------

json_data = {}
for i in range(1024):
    str_key = 'long key' * i
    str_value = 'long value' * i
    json_data[str_key] = str_value
    
with open('test_long-key.json', 'w') as f:
    json.dump(json_data, f)
    
# -----------
# test json: many arrays
# -----------

json_data = []
for i in range(1024):
    json_data.append(i)
    json_data.append(str(i))
    json_data.append({str(i): str(i)})
    json_data.append([])
    
with open('test_many-arrays.json', 'w') as f:
    json.dump(json_data, f)
    