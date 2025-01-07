#!/usr/bin/env python 
import json
import random

# Path to your JSON file
filename = '../pol.json'

try:
    # Read JSON data
    with open(filename, 'r') as file:
        data = json.load(file)
    
    # Iterate through each record
    for record in data:
        # Check if 'gap of years' exists and is a string (unconverted)
        if 'gap of years' in record and isinstance(record['gap of years'], str):
            # Split, strip, and convert to integers
            start_year, end_year = map(lambda x: int(x.strip()), record['gap of years'].split('-'))
            # Replace the string with a tuple
            record['gap of years'] = (start_year, end_year)
        
        # Check if "author's name" exists and is a string, then add or update the surname
        if "author's name" in record and isinstance(record["author's name"], str):
            surname = record["author's name"].strip().split(' ')[-1].strip()
            # Add or update the 'surname' field in the record
            record['surname'] = surname
        
        # Check if "year of release" is a string (unconverted)
        if "year of release" in record and isinstance(record["year of release"], str):
            # Convert to integer
            record["year of release"] = int(record["year of release"])
        
        # Check if "award" is a string (unconverted)
        if "Awards" in record and isinstance(record["Awards"], str):
            #Generate random number for awards (0-2)
            #random_number = random.randint(0, 2)
            #record["Awards"] = random_number
            # Convert to integer
            record["Awards"] = int(record["Awards"])

    # Write modified data back to JSON
    with open(filename, 'w') as file:
        json.dump(data, file, indent=4)

    print(f"Updated data saved in {filename}")

except FileNotFoundError:
    print("File not found. Please check the file path.")
except json.JSONDecodeError:
    print("Invalid JSON file. Please check the file's structure.")
except Exception as e:
    print(f"An error occurred: {e}")
