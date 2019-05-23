import base_pb2 as base
import phenopackets_pb2 as phenopackets
from google.protobuf import json_format
from google.protobuf.json_format import Parse, MessageToJson
#from google.protobuf import JsonFormat
import json

### Input phenopacket from JSON file


path="../Gebbia-1997-ZIC3.json"
with open(path,'r') as f:
    packet=Parse(message=phenopackets.Phenopacket(),text=f.read())
    #print (packet)

print("Reading phenopacket from {}".format(path))
print("Subject id: {}".format(packet.subject.id))
print("dtaypte",type(packet.subject))
indiv = packet.subject
print(indiv.id)



print(indiv.age_at_collection.age)
