# Segment Tree Implementation for Interval and Point Queries

## Overview
The Segment Tree Implementation for Interval and Point Queries is a custom Python solution designed to efficiently manage and query interval data. Developed through advanced programming techniques, this codebase facilitates rapid point and interval queries across extensive datasets. It's particularly beneficial for applications in data science, historical research, and any field requiring precise interval analysis. This implementation stands out for its effective use of Segment Trees, making it an essential asset for processing and analyzing complex interval data with speed and accuracy.

## Table of Contents
- [Segment Tree Implementation for Interval and Point Queries](#segment-tree-implementation-for-interval-and-point-queries)
  - [Overview](#overview)
  - [Table of Contents](#table-of-contents)
  - [Installation](#installation)
  - [Usage](#usage)
    - [Basic Usage](#basic-usage)
  - [Code Documentation](#code-documentation)
    - [Modules](#modules)
    - [Functions](#functions)
    - [Classes](#classes)

## Installation
Βεβαιωθείτε ότι η Python 3 είναι εγκατεστημένη στον υπολογιστή σας.

## Usage

### Basic Usage
Το σύστημα λειτουργεί με interval data που παρέχονται σε μορφή JSON. Κάθε interval θα πρέπει να περιλαμβάνει ένα σημείο έναρξης και λήξης (υπό το κλειδί "gap of years") και ένα σχετικό "author's name". Αφού φορτώσετε τα δεδομένα σας, το σύστημα επιτρέπει διάφορους τύπους ερωτημάτων:

- **Point Query**: Προσδιορίζει όλα τα διαστήματα που περιέχουν ένα δεδομένο σημείο.
- **Range Query**: Εντοπίζει όλα τα διαστήματα που επικαλύπτονται με ένα καθορισμένο εύρος.

## Code Documentation

### Modules
- json: Για να φορτωσει interval data απο ενα JSON file.

### Functions
Οι σημαντικές λειτουργίες του script περιλαμβάνουν:
- `build_tree()`: Κατασκευάζει το segment tree από ένα αρχικό σύνολο διαστημάτων.
  - **Purpose**: Κατασκευάζει τη βασική δομή του segment tree δημιουργώντας κόμβους για κάθε τμήμα εντός του συνολικού εύρους δεδομένων.
  - **Process**: Προσδιορίζει τις ελάχιστες και μέγιστες τιμές σε όλα τα διαστήματα για τον καθορισμό του εύρους δεδομένων. Στη συνέχεια, διαιρεί αναδρομικά αυτό το εύρος για να δημιουργήσει ένα δυαδικό δέντρο όπου κάθε κόμβος αντιπροσωπεύει ένα τμήμα των δεδομένων.
  - **Outcome**: Ένα πλήρως κατασκευασμένο segment tree όπου κάθε κόμβος αντιπροσωπεύει ένα συγκεκριμένο εύρος, έτοιμο για εισαγωγή διαστημάτων και αναζήτηση.

- `insert()`: Προσθέτει ένα νέο διάστημα στο segment tree.
  - **Purpose**: Προσθέτει ένα νέο διάστημα στο segment tree, διασφαλίζοντας ότι το δέντρο αντιπροσωπεύει με ακρίβεια όλα τα διαστήματα στο σύνολο δεδομένων.
  - **Process**: Η συνάρτηση διατρέχει το δέντρο για να εντοπίσει κόμβους των οποίων τα τμήματα καλύπτονται από το νέο διάστημα. Στη συνέχεια αποθηκεύει το διάστημα σε κάθε κόμβο που καλύπτεται πλήρως από αυτό. Για τις μερικές επικαλύψεις, η εισαγωγή συνεχίζεται αναδρομικά για να εξασφαλιστεί η πλήρης κάλυψη.
  - **Outcome**: Το διάστημα αποθηκεύεται σε όλους τους σχετικούς κόμβους του δέντρου, επιτρέποντας την αποδοτική υποβολή ερωτημάτων με βάση αυτό το ενημερωμένο σύνολο δεδομένων.

- `query()`: Εκτελεί ερωτήματα σημείου ή εύρους στο δέντρο.
  - **Purpose**: Εκτελεί ερωτήματα στο segment tree για την εύρεση διαστημάτων με βάση ένα συγκεκριμένο σημείο ή εύρος.
  - **Point Query Process**: Διατρέχει το δέντρο, συλλέγοντας διαστήματα από κόμβους των οποίων τα τμήματα περιλαμβάνουν το σημείο του ερωτήματος. Αυτό περιλαμβάνει τον έλεγχο των διαστημάτων σε κάθε σχετικό κόμβο και συνεχίζει προς τα κάτω σε κόμβους-παιδιά, όπως είναι απαραίτητο.
  - **Interval Query Process**: Ψάχνει για κόμβους που επικαλύπτονται με το εύρος του ερωτήματος, συλλέγοντας διαστήματα από αυτούς τους κόμβους και ελέγχοντας αναδρομικά τους κόμβους-παιδιά για πρόσθετες επικαλύψεις.
  - **Outcome**: Επιστρέφει έναν κατάλογο διαστημάτων που ικανοποιούν τα κριτήρια του ερωτήματος, επιτρέποντας στους χρήστες να έχουν γρήγορη πρόσβαση σε δεδομένα που σχετίζονται με το ερώτημά τους.

### Classes
Η αρχιτεκτονική του συστήματος περιλαμβάνει διάφορες κλάσεις:
- `SegmentTreeNode`: Μια κλάση που αναπαριστά έναν κόμβο στο segment tree, κρατώντας το εύρος του και όλα τα διαστήματα πλήρους κάλυψης.
- `SegmentTree`: Διαχειρίζεται τη συνολική δομή του δέντρου, συμπεριλαμβανομένης της κατασκευής, της εισαγωγής διαστημάτων και της εκτέλεσης ερωτημάτων.
