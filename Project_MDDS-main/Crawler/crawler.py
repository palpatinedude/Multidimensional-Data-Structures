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