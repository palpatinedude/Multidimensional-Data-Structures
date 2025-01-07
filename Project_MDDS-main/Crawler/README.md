# Crawler, Change Data, Get Hash Size

## Overview
Ο φάκελος αυτός έχει δημιουργηθεί για την μεταφορά και μετατροπή των δεδομένων για επιστήμονες όπου αυτή κριθεί απαραίτητη σε json μορφή, όπως επίσης για την εύρεση και μεταφόρα μηκών δεδομένων που θα χρησιμοποιήσουμε στην συνέχεια σε ένα environmental αρχείο.

## Table of Contents
- [Crawler, Change Data, Get Hash Size](#crawler-change-data-get-hash-size)
  - [Overview](#overview)
  - [Table of Contents](#table-of-contents)
  - [Installation & Requirements](#installation--requirements)
  - [Usage](#usage)
  - [Code Documentation](#code-documentation)
    - [Modules](#modules)
    - [Crawler](#crawler)
    - [Code for Crawler](#code-for-crawler)
    - [Code for Change Data](#code-for-change-data)
    - [Code for Get Hash Size](#code-for-get-hash-size)

## Table of Contents

## Installation & Requirements
Τα requirements για την εκτελσεση του κώδικα είναι τα εξής:
- Εγκατάσταση της Python
  - Για την εγκατάσταση της Python, ακολουθήστε τις οδηγίες που βρίσκονται στον παρακάτω σύνδεσμο: [Python Installation](https://www.python.org/downloads/)
  - Εγκατάσταση του pip
    - Μετά την εγκατάσταση της Python, εγκαταστήστε το pip ακολουθώντας τις οδηγίες που βρίσκονται στον παρακάτω σύνδεσμο: [Pip Installation](https://pip.pypa.io/en/stable/installation/)
  - Εγκατάσταση του lib `random`
    - Μετά την εγκατάσταση του pip, εγκαταστήστε το lib `random` μέσω του pip με την εντολή: `pip install random2`
- Εγκατάσταση του lib `requests`
    - Μετά την εγκατάσταση του pip, εγκαταστήστε το lib `BeautifulSoup` μέσω του pip με την εντολή: `pip install beautifulsoup4`

## Usage

Για την εκτέλεση του κώδικα ακολουθήστε τα παρακάτω βήματα:
- Για το αρχείο change_data τρεξτε τον κώδικα με την εντολή `./change_data.py`
- Για το αρχείο crawler τρεξτε τον κώδικα με την εντολή `./crawler.py`
- Για το αρχείο `get_hash_size` τρεξτε τον κώδικα με την εντολή `./get_hash_size.py`

## Code Documentation

### Modules
- `change_data.py` : Περιέχει τον κώδικα για την μετατροπή δεδομένων όπου αυτό κριθεί απαραίτητο.
- `crawler` : Περιέχει τον κώδικα για την μεταφορά των δεδομένων που μας ενδιαφέρουν για computer scientists από την ιστοσελίδα [DBLP_Records for computer_scientists](https://dblp.org.pers/) και την δημιουργία ενός json αρχείου `records.json` όπου αποθηκεύονται τα δεδομένα για κάθε επιστήμονα ξεχωριστά.
- `get_hash_size.py` : Περιέχει τον κώδικα για την δημιουργία ενός `.env`file στο οποίο περιέχονται τα μήκη των DBLP_Records και Surnames για να μπορούμε όπου χρειαστεί να μετατρέψουμε τα strings DBLP_Records και Surnames σε integers (hash).

### Crawler

Εδώ θα αναλύσουμε λίγο μόνο τον κώδικα του Crawler εφόσον ο κώδικας του change_data.py και get_hash_size.py είναι αρκετά απλός και δεν υπάρχει ανάγκη για περεταίρω ανάλυση.

- Βιβλιοθήκη `requests`: χρησιμοποιείτε για αποστολή αιτήματος σε μια ιστοσελίδα για άδεια χρήσης των δεδομένων της.

- Με την βοήθεια της βιβλιοθήκης `beautifulsoup4` αναλύουμε HTML και XML εγγραφές. 

- Δημιουργούμε αίτηση προς την ιστοσελίδα [DBLP_Records for computer scientists](https://dblp.org.pers/) και αποθηκεύουμε τα δεδομένα της σε μια μεταβλητή `response = requests.get(url)` και στην συνέχεια χρησιμοποιώντας την βιβλιοθήκη `BeautifulSoup` βρίσκουμε τα tags και έπειτα τα URL που αντιστοιχούν στις ιστοσελίδες για τον κάθε επιστήμονα ξεχωριστά. Στην συνέχεια βρίσκουμε τα tags για τα δεδομένα που μας ενδιαφέρουν και αποθηκεύουμε τα δεδομένα που αντιστοιχούν στα tags σε ένα json αρχείο `records.json` όπου γίνεται έλεγχος εάν οι εγγραφές έχουν μεταφερθεί σωστά και τα μεταφέρουμε στο αρχείο `poll.son` όπου υπάρχουν πλέον τα δεδομένα των computer scientists για να τα χρησιμοποιήσουμε στα επόμενα ερωτήματα.

- Αρχείο `records.json`: Περιέχει τα δεδομένα των computer scientists.

- Τα δεδομένα αυτά είναι:

    1.`author's name` 2.`gap of year` 3.`year of release` 4.`DBLP_Record` 5.`Awards` 6.`kind` 7.`co-author` 8.`surname`


### Code for Crawler

```python
import json
import random
import requests 
from bs4 import BeautifulSoup 
 
# Define the URL to scrape 
url = 'https://dblp.org/pers/' 
 
# Send a GET request to the URL and get the response 
response = requests.get(url) 
 
# Create a BeautifulSoup object by passing in the response content and 'html.parser' as the parser 
soup = BeautifulSoup(response.content, 'html.parser') 

#Get tags
html_tags = soup.find_all('div', class_='columns hide-body')

#Creating list for every author link
links = []

#Find every author_data_link and extract the informations to json file
for tag in html_tags:
    for a_tag in tag.find_all('a', href=True):
        #Get link for every author
        author_data_link = a_tag['href']
        print(author_data_link)
        #links.append(a_tag['href'])
        #Visit the author link
        author_data_response = requests.get(author_data_link)
        author_soup = BeautifulSoup(author_data_response.content, 'html.parser')
        #Get tags
        author_tags = author_soup.find_all('span', class_='this-person')
        coauthor_tags = author_soup.find_all('cite', class_='data tts-content', itemprop='headline')
        title_tags = author_soup.find_all('span', class_='title')
        year_tags = author_soup.find_all('span', itemprop='datePublished')
        gapofyears_tags = author_soup.find_all('header', class_="hide-head h2",)
        dblp_tags = author_soup.find_all('li', class_='select-on-click')

        #Get author names
        authors = [author.text.strip() for author in author_tags]

        #Get coauthor names
        coauthors = [coauthor.find('span', itemprop='author').text.strip() for coauthor in coauthor_tags]

        #Get publication title
        titles = [title.text.strip() for title in title_tags]

        #Get year of publication
        dblp_records = [dblp.text.strip() for dblp in dblp_tags]
        #Remove first dblp because is the general one
        dblp_records = dblp_records[1:]

        #Get year of publication
        years = [year.text.strip() for year in year_tags]

        #Get gap of years
        gapofyears = [gapyear.find('h2').text.strip() for gapyear in gapofyears_tags]
        #Remove first and last tag because are irrelevant
        gapofyears = gapofyears[1:]
        gapofyears = gapofyears[:-1]

        #Split the gap of years and if gap of years[1] is today change it
        gapofyearsCorrect = []
        for gap in gapofyears:
            #Split the string by ' - ' and strip whitespace
            splt = gap.split(' – ')
            start_year = int(splt[0].strip())
            if splt[1] == "today":
                end_year = 2024
            else:   
                end_year = int(splt[1].strip())
            gapofyearsCorrect.append((start_year, end_year))
        print(gapofyearsCorrect)


        #for i in range(len(authors)):
        #    print("Authors:", authors[i])
        #    print ("Coauthors:", coauthors)
        #    print("Title:", titles[i])
        #    print("DBLP_Record:", dblp_records[i])
        #    print("Gap of Years:", gapofyears)
        #    print("Year:", years[i])

        #json list
        records = []
        #Create records for the json file
        for i in range(len(authors)):
            #Find the correct co-authors for each author
            coauthors = []
            for coauthor in coauthor_tags[i].find_all('span', itemprop='author'):
                co_author = coauthor.text.strip()
                #Check if authors name is in coauthors and ignore it
                if co_author != authors[i]:
                    coauthors.append(co_author)
            #Find the correct gap of years for each record
            for g in gapofyearsCorrect:
                if int(years[i]) >= g[0] and int(years[i])<= g[1]:
                    goy = g
            #Find the correct kind
            kind = dblp_records[i].split('/')[0].strip()
            if kind == "conf":
                kind = "conference and workshop papers"
            elif kind == "books":
                kind = "books and these"
            elif kind == "journals":
                kind = "Journal articles"
            #Create the record
            record = {
                "author's name": authors[i],
                "title": titles[i],
                "gap of years": goy,
                "year of release": years[i],
                "DBLP_Record": dblp_records[i],
                "Awards": random.randint(0,2),
                "kind": kind,
                "co-author": coauthors,
                "surname": authors[i].strip().split(' ')[-1].strip(),
            }
            records.append(record)

        #Path to JSON file
        filename = "records.json"

        #Load existing data from the JSON file if it exists
        try:
            with open(filename, 'r') as file:
                data = json.load(file)
        except FileNotFoundError:
            data = []
        except json.JSONDecodeError:
            data = []

        #Add only new records to the JSON file
        #Not working cause of awards random
        for record in records:
            if record not in data:
                data.append(record)

        #Write the updated data to the JSON file
        with open(filename, 'w') as file:
            json.dump(data, file, indent=4)

        print("Data has been successfully appended to", filename)

# Print the extracted links
print(links)
```


### Code for Change Data

```python
#!/usr/bin/env python 
import json
import random

# Path to your JSON file
filename = '../../pol.json'

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

```


### Code for Get Hash Size

```python
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

```