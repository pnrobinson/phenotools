import base_pb2 as base
import phenopackets_pb2 as phenopackets
from google.protobuf import json_format
#from google.protobuf import JsonFormat
import json

### Input phenopacket from JSON file
phenopacket=phenopackets.Phenopacket();

path="../Gebbia-1997-ZIC3.json"
with open(path,'r') as f:
    json_string=json.load(f)
#print(json_string)


json_format.Parse(json_string,phenopacket)
