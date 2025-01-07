# Interval Tree Implementation for Interval and Point Queries

## Overview
Αυτό το πρόγραμμα υλοποιεί ένα interval tree, έναν ειδικό τύπο δυαδικού δέντρου αναζήτησης που έχει σχεδιαστεί για την αποτελεσματική εύρεση όλων των intervals που επικαλύπτονται με οποιοδήποτε δεδομένο διάστημα ή σημείο. Είναι ιδιαίτερα χρήσιμο στην υπολογιστική γεωμετρία και σε εφαρμογές όπως ο χρονοπρογραμματισμός, όπου συχνά πρέπει να αναζητήσει κανείς αλληλεπικαλυπτόμενα διαστήματα. Η υλοποίηση περιλαμβάνει λειτουργίες για τη δημιουργία του δέντρου από ένα σύνολο intervals, την εισαγωγή νέων interval στο tree και την εκτέλεση ερωτημάτων τόσο ανά σημείο (εύρεση όλων των intervals που περιέχουν ένα δεδομένο σημείο) όσο και ανά διάστημα (εύρεση όλων των interval που τέμνονται με ένα δεδομένο διάστημα).

## Table of Contents
- [Interval Tree Implementation for Interval and Point Queries](#interval-tree-implementation-for-interval-and-point-queries)
  - [Overview](#overview)
  - [Table of Contents](#table-of-contents)
  - [Installation](#installation)
  - [Usage](#usage)
    - [Basic Usage](#basic-usage)
    - [Advanced Features](#advanced-features)
  - [Code Documentation](#code-documentation)
    - [Modules](#modules)
    - [Functions](#functions)
    - [Classes](#classes)

## Installation
Αυτό το project απαιτεί Python 3. Βεβαιωθείτε ότι η Python είναι εγκατεστημένη στο σύστημά σας.

## Usage

### Basic Usage

Για να χρησιμοποιήσετε αυτή την υλοποίηση του interval treee, θα πρέπει να έχετε το JSON αρχείο `pol.json`. Το script διαβάζει αυτό το αρχείο, δημιουργεί το interval tree και σας επιτρέπει να εκτελείτε ερωτήματα σημείων και διαστημάτων.

```python
# Example of performing a point query
point_query_result = interval_tree.query(point=1995)
print("Point Query Result:", point_query_result)

# Example of performing an interval query
interval_query_result = interval_tree.interval_query(start=1995, end=1997)
print("Interval Query Result:", interval_query_result)
```

### Advanced Features

Η Advanced χρήση περιλαμβάνει την τροποποίηση του δέντρου διαστημάτων με την εισαγωγή νέων διαστημάτων και τη χρήση του δέντρου για σύνθετα ερωτήματα που περιλαμβάνουν πολλαπλά διαστήματα ή σημεία.

## Code Documentation

### Modules
- json: Για να φορτωσει interval data απο ενα JSON file.

### Functions
- main(): Το main entry point του script, που δείχνει πώς να δημιουργείται το δέντρο και να εκτελούνται queries.

### Classes
- Node: Αντιπροσωπεύει ένα node στο interval tree.
- IntervalTree: Υλοποιεί τη δομή του interval tree και τις σχετικές μεθόδους.