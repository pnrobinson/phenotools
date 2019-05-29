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
indiv = packet.subject
print(indiv.id)
if indiv.age_at_collection and indiv.age_at_collection.age:
    print("Age: {}".format(indiv.age_at_collection.age))
else:
    print("Could not find age")
if indiv.sex:
    sx = base.Sex.Name(indiv.sex)
    print(sx)
else:
    print("Could not find sex element")
for pt in packet.phenotypes:
    term=pt.type
    print("{} [{}]".format(term.label,term.id))