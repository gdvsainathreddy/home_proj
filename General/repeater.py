import json

# Define the base JSON object
base_json = {
    "accessory": "HTTP-SWITCH",
    "name": "ht<num>",
    "switchType": "stateful",
    "onUrl": "http://192.168.0.152/gpio<num>/on/",
    "offUrl": "http://192.168.0.152/gpio<num>/off/",
    "statusUrl": "http://192.168.0.152/gpio<num>/status/"
}

# Generate 512 copies with <num> replaced
json_data_list = []

for i in range(512):
    current_json = base_json.copy()
    current_json["name"] = current_json["name"].replace("<num>", str(i))
    current_json["onUrl"] = current_json["onUrl"].replace("<num>", str(i))
    current_json["offUrl"] = current_json["offUrl"].replace("<num>", str(i))
    current_json["statusUrl"] = current_json["statusUrl"].replace("<num>", str(i))

    json_data_list.append(current_json)

# Write the JSON data to a file
with open('output.json', 'w') as outfile:
    json.dump(json_data_list, outfile, indent=4)
