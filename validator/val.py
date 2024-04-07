import json
from jsonschema import validate
from jsonschema.exceptions import ValidationError

with open('exemple_1.json') as f:
    document = json.load(f)

with open('shema.json') as f:
    schema = json.load(f)
    print("Validation est Okay")

try:
    validate(instance=document, schema=schema)

except ValidationError as e:
    print("Erreur de validation")
    print(f"Erreur: {e.message}")