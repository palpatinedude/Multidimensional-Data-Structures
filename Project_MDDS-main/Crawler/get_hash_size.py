#!/usr/bin/env python 
import json

# Path to your JSON file
filename = '../pol.json'

try:
    # Read JSON data
    with open(filename, 'r') as file:
        data = json.load(file)

    # list of all existing dblp records
    dblp_records = []

    # list of all existing surnames
    surnames = []

    # Iterate through each record
    for record in data:
        # Check if the record has a dblp key
        if 'DBLP_Record' in record:
            # Add the record to the list of dblp records
            dblp_records.append(record['DBLP_Record'])
        
        if 'surname' in record:
            surnames.append(record['surname'])

    # remove duplicates
    dblp_records = list(set(dblp_records))
    surnames = list(set(surnames))

    print(f"DBLP records: {len(dblp_records)}")
    print(f"Surnames: {len(surnames)} {surnames}")

    # open env file to write the hash
    with open('../.env', 'w') as file:
        file.write(f"DBLP_RECORDS_LENGTH={len(dblp_records)}\n")
        file.write(f"SURNAMES_LENGTH={len(surnames)}\n")

except FileNotFoundError:
    print("File not found. Please check the file path.")
except json.JSONDecodeError:
    print("Invalid JSON file. Please check the file's structure.")
except Exception as e:
    print(f"An error occurred: {e}")
