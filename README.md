ITTIA C++ Phone Book Example
============================

A portable console application to store contacts and their phone numbers, as on
a mobile handset. This example highlights features available through the ITTIA
DB C++ API, including:

* File and memory storage
* Client/server shared access
* Executing SQL statements
* Iterating over table cursors
* Transaction control
* Importing and exporting binary large objects (BLOBs)

This example requires the ITTIA DB SQL embedded database library, which can be
obtained from:

http://www.ittia.com/products/evaluation


Source Files
------------

The phone book application uses one of `phonebook.cpp` or `phonebook_sql.cpp`
and all other source files.

**`phonebook.h`**

Constants, data structures, and function declarations for the C++ phone book
data access layer.

**`phonebook_console.cpp`**

Console-based user interface to interact with the phone book database.

**`phonebook.cpp`**

Data access layer for the phone book database, implemented with table cursors
and no dependency on SQL.

**`phonebook_sql.cpp`**

Data access layer for the phone book database, implemented with SQL statements.


Database Schema
---------------

Database file name: `phone_book.db`

**`contact` table**

A list of contacts.

Field          | Data Type      | Description
-------------- | -------------- | -----------------------------------------
`id`           | `uint64`       | unique person identifier
`name`         | `nvarchar(50)` | contact's name encoded in Unicode
`ring_id`      | `uint64`       | ring tone to play when this contact calls
`picture_name` | `varchar(50)`  | name of the picture file
`picture`      | `blob`         | a picture of the contact

Index     | Type        | Columns  | Description
--------- | ----------- | -------- | -------------------------------------------
`by_id`   | primary key | `(id)`   | find a contact by ID and enforce uniqueness
`by_name` | multiset    | `(name)` | find a contact by name

**`phone_number` table**

Phone numbers associated with each contact.

Field        | Data Type     | Description
------------ | ------------- | ------------------
`contact_id` | `uint64`      | associated contact 
`number`     | `varchar(20)` | phone number
`type`       | `uint64`      | device type
`speed_dial` | `sint64`      | speed dial number

Index           | Type        | Columns        | Description
--------------- | ----------- | -------------- | --------------------------
`by_contact_id` | multiset    | `(contact_id)` | find by associated contact

**`contact_id` sequence**

Generates surrogate identifiers for the contact.id field.
