/**************************************************************************/
/*                                                                        */
/*      Copyright (c) 2005-2014 by ITTIA L.L.C. All rights reserved.      */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of ITTIA     */
/*  L.L.C.  All rights, title, ownership, or other interests in the       */
/*  software remain the property of ITTIA L.L.C.  This software may only  */
/*  be used in accordance with the corresponding license agreement.  Any  */
/*  unauthorized use, duplication, transmission, distribution, or         */
/*  disclosure of this software is expressly forbidden.                   */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of ITTIA L.L.C.                                       */
/*                                                                        */
/*  ITTIA L.L.C. reserves the right to modify this software without       */
/*  notice.                                                               */
/*                                                                        */
/*  info@ittia.com                                                        */
/*  http://www.ittia.com                                                  */
/*                                                                        */
/*                                                                        */
/**************************************************************************/


/** @file
 *
 * Command line example program demonstrating the ITTIA DB C++ API
 */

#include "phonebook.h"
#include "dbs_error_info.h"

#include <iostream>
#include <stdio.h>

#ifdef __embedded_cplusplus
#define cerr cout
#else
using std::cerr;
using std::cout;
using std::endl;
#endif


/**
 * Helper function to print error messages.
 */
int print_error(int rc)
{
    if (DB_FAILED(rc)) {
        dbs_error_info_t info = dbs_get_error_info( rc );
        cerr << "ERROR " << info.name << ": " << info.description << endl;
    }
    return rc;
}

/** 
 * Create database tables, assuming an empty database has been created.
 * 
 * @return database error code
 *	 
 * Demonstrates:
 * - DB_SUCCESS macro
	 * - DB_NOERROR status code
	 */
int PhoneBook::create_tables(bool with_picture)
{
	if (DB_SUCCESS(create_table_contact(with_picture)) &&
			DB_SUCCESS(create_table_phone_number())) {
		// Success
		return DB_NOERROR;
	} else {
		// Table error
		return DB_ETABLE;
	}
}

/**
 * Create the table "contact", which lists contacts in the phone book.
 *
 * Demonstrates:
 * - defining table schema: fields and indexes
 * - unique and non-unique indexes
 * - add_datatype() functions return FieldDesc so they can be chained together
 * - add_index() functions return IndexDesc so they can be chained together
 */
int PhoneBook::create_table_contact(bool with_picture)
{
	db::FieldDescSet fields;
	db::IndexDescSet indexes;

	// Create fields

	// Unique contact id number
	fields.add_uint("id");
	// Contacts's name
	fields.add_wstring("name", 50);
	// Ring tone to use when this contact calls
	fields.add_uint("ring_id", sizeof(db_uint), true);
	// Picture name
	fields.add_string("picture_name", 50, true);
    if (with_picture) {
	    // Picture of this contact
	    fields.add_blob("picture");
    }

	// Create a primary key index named "$PK" on the field "id"
	indexes.add_index("by_id", db::DB_PRIMARY)
				 .add_field("id");

	// Create an index on name
	indexes.add_index("by_name", db::DB_MULTISET)
				 .add_field("name");

	return db.create_table("contact", fields, indexes);
}

/**
 * Create the table "phone_number", which lists all known telephone numbers.
 */
int PhoneBook::create_table_phone_number()
{
	db::FieldDescSet fields;
	db::IndexDescSet indexes;
    db::ForeignKeyDescSet foreign_keys;

	// Foreign key into the "contact" table
	fields.add_uint("contact_id");
	// The telephone number, stored as a string
	fields.add_string("number", 20);
	// The type of device
	fields.add_uint("type");
	fields.add_sint("speed_dial", sizeof(db_sint), true);

	indexes.add_index("by_contact_id", db::DB_MULTISET)
				 .add_field("contact_id");

    foreign_keys.add_foreign_key("contact_ref", "contact", DB_FK_MATCH_SIMPLE, DB_FK_ACTION_RESTRICT, DB_FK_ACTION_RESTRICT)
        .add_field("contact_id", "id");

	return db.create_table("phone_number", fields, indexes, foreign_keys);
}

/**
 * Create sequences. Sequences are used to generate unique identifiers.
 *
 * Demonstrates:
 * - defining sequences
 */
int PhoneBook::create_sequences()
{
	if (DB_SUCCESS(db.create_sequence("contact_id", 1))) {
		// Success
		return DB_NOERROR;
	} else {
		// Sequence error
		return DB_ESEQ;
	}
}

/** 
 * Open the database if it exists, otherwise create an empty database.
 * 
 * @return database error code
 *
 * Demonstrates:
 * - opening a database
 * - the DB_FAILED macro
 */
int PhoneBook::open_database(int file_mode, const char* database_name)
{
	// Return code
	int rc;

	// Default database storage mode options
	db::StorageMode mode;
    mode.file_mode = file_mode;

	rc = db.open(database_name, mode);

	if (DB_FAILED(rc)) {
		cerr << "Error opening database."<< endl;
        print_error(rc);
		return rc;
	}

	return DB_NOERROR;
}

/**
 * Create an empty database.
	 *
	 * Demonstrates:
	 * - creation of an empty database
	 * - StorageMode parameter
	 */
int PhoneBook::create_database(int file_mode, const char* database_name)
{
	int rc;
	db::StorageMode mode;
    mode.file_mode = file_mode;
    if (file_mode == db::DB_MEMORY_STORAGE) {
        mode.memory_storage_size = MEMORY_STORAGE_SIZE;
        cout << "Creating " << mode.memory_storage_size << " byte memory storage." << endl;
    }

	// Create a new empty database, overwriting existing files
	rc = db.create(database_name, mode);

	if (DB_FAILED(rc)) {
		cerr << "Error creating new database." << endl;
        print_error(rc);
		return rc;
	}

	rc = create_tables(file_mode != db::DB_MEMORY_STORAGE);
	if (DB_FAILED(rc)) {
		cerr << "Error creating tables." << endl;
        print_error(rc);
		return rc;
	}

	rc = create_sequences();
	if (DB_FAILED(rc)) {
		cerr << "Error creating sequences." << endl;
        print_error(rc);
		return rc;
	}

	return rc;
}

/** 
 * Close the database.
 * 
 * @return database error code
 */
int PhoneBook::close_database()
{
	return db.close();
}

/**
 * Insert a contact into the database.
 *
 * Demonstrates:
 * - use of a sequence
 * - opening of a table
 * - insert mode
 * - assigning data to a row
 * - posting data to the database
 * - closing a table
 * - inserting data into a BLOB field
	 */
db_uint PhoneBook::insert_contact(const wchar_t *name, db_uint ring_id, const char *picture_name)
{
	db::Table t;
	db::Sequence id_sequence;
	db_uint id;

	id_sequence.open(db, "contact_id");
	print_error(id_sequence.get_next_value(id));

	t.open(db, "contact");

	// Put table in insert mode
	t.insert();
	// Store values for each field in a temporary buffer
	t["id"] = id;
	t["name"] = name;
	t["ring_id"] = ring_id;
	t["picture_name"] = picture_name;
	// Post the row data. This does not commit the current transaction.
	if (DB_FAILED(print_error(t.post())))
		id = 0;

	// Open picture file
	FILE *picture_file;
	if ((picture_file = fopen(picture_name, "rb")) != NULL) {
						
		// Prepare BLOB variables
		int picture_field = t.find_field("picture");
		int num_chunks = 0;
		int bytes_read = 0;
		const db_len_t data_size = 256;
		char data[data_size];

		// Store picture into BLOB field
		while((bytes_read = (int)fread(data, 1, data_size, picture_file)) > 0)
		{
			t.write_blob (picture_field, data_size * num_chunks, data, bytes_read);
			num_chunks++;
		}
		fclose(picture_file);
	
	} else {
		cerr << "Cannot open " << picture_name << endl;
	}

	t.close();

	return id;
}

/**
 * Insert a phone entry into the database.
 */
void PhoneBook::insert_phone_number(db_uint contact_id, const char *number, PhoneNumberType type, db_sint speed_dial)
{
	db::Table t;

	t.open(db, "phone_number");

	t.insert();
	t["contact_id"] = contact_id;
	t["number"] = number;
	t["type"] = type;
	t["speed_dial"] = speed_dial;
	if (DB_FAILED(print_error(t.post())))
		cerr << "Could not enter new phone number" << endl;

	t.close();
}

/**
 * Update an existing contact's name.
	 *
	 * Demonstrates:
	 * - searching for existence of a record using an index
	 * - edit mode
	 */
void PhoneBook::update_contact_name(db_uint id, const wchar_t *newname)
{
	db::Table contact;

	contact.open(db, "contact");

	// Sort with the "$PK" index to avoid a table scan.
	contact.set_sort_order("$PK");
    // Filter by the "id" column.
    contact.begin_filter(db::DB_SEEK_EQUAL);
	contact["id"] = id;
	if (DB_SUCCESS(print_error(contact.apply_filters()))) {
		// Edit the current row
		contact.edit();
		contact["name"] = newname;
		contact.post();
	} else {
		cerr << "Could not find contact with id " << (long) id << endl;
	}

	contact.close();
}

/**
 * Remove contact record from the database.
	 *
 * Demonstrates:
	 * - deleting a record
	 * - parent/child removal
	 * - range search loop using seek_next()
	 * - seek_next() returns OK on end, so must explicitly check for is_eof()
 */
void PhoneBook::remove_contact(db_uint id)
{
	db::Table contact;
	db::Table phone_number;

	contact.open(db, "contact");

	// Sort with the "$PK" index to avoid a table scan.
	contact.set_sort_order("$PK");
    // Filter by the "id" column.
	contact.begin_filter(db::DB_SEEK_EQUAL);
	contact["id"] = id;
	if (DB_SUCCESS(print_error(contact.apply_filters()))) {
		db_uint id = contact["id"].as_int();

        // Optimization: prevent others from reading this contact while its
        // phone numbers are removed.
        contact.lock_row(db::DB_LOCK_EXCLUSIVE);

        // Remove related telephone numbers

		phone_number.open(db, "phone_number");
		phone_number.set_sort_order("by_contact_id");

		// Filter phone numbers by the "contact_id" column.
		phone_number.begin_filter(db::DB_SEEK_EQUAL);
		phone_number["contact_id"] = id;
		phone_number.apply_filters();

        // Remove all matching phone numbers.
        for (phone_number.seek_first(); !phone_number.is_eof(); phone_number.seek_next())
			phone_number.remove();

		// Remove the current contact
		contact.remove();

		phone_number.close();
	} else {
		cerr << "Could not find contact with id " << (long) id << endl;
	}

	contact.close();
}

/**
 * Briefly list all contacts in the database.
 */
void PhoneBook::list_contacts_brief()
{
	db::Table contact;
	
	contact.open(db, "contact");
	contact.set_sort_order("by_name");

	for (contact.seek_first(); !contact.is_eof(); contact.seek_next()) {
		db_uint id = contact["id"].as_int();
		db::WString name = contact["name"].as_wstring();
        char name_mbs[50];

        wcstombs(name_mbs, name.c_str(), sizeof name_mbs/sizeof name_mbs[0]);

		cout << (long) id << '\t';
		cout << name_mbs << endl;
	}

	contact.close();
}

/**
 * List all contacts in the database with full phone numbers
 *
 * Demonstrates:
 * - parent/child relationships
 */
void PhoneBook::list_contacts(int sort)
{
	db::Table contact;
	db::Table phone_number;
	
	contact.open(db, "contact");
	contact.set_sort_order("by_name");

    db::IndexFieldSet sort_fields;
    switch (sort) {
        case 0:
            sort_fields.add("id");
            break;
        case 1:
            sort_fields.add("name");
            break;
        case 2:
            sort_fields.add("ring_id");
            sort_fields.add("name");
            break;
    }
    contact.sort(sort_fields);

	for (contact.seek_first(); !contact.is_eof(); contact.seek_next()) {
		db_uint id = contact["id"].as_int();

		db::WString name = contact["name"].as_wstring();
        char name_mbs[50];
		db_uint ring_id = contact["ring_id"].as_int();
		db::String picture_name = contact["picture_name"].as_string();
        wcstombs(name_mbs, name.c_str(), sizeof name_mbs/sizeof name_mbs[0]);

		// Output the contact's name and ring tone
		cout << "Id: " << (long) id << endl;
		cout << "Name: " << name_mbs << endl;
        if (!contact["ring_id"].is_null())
    		cout << "Ring tone id: " << (int) ring_id << endl;
        if (!contact["picture_name"].is_null())
    		cout << "Picture name: " << picture_name.c_str() << endl;

		phone_number.open(db, "phone_number");
		phone_number.set_sort_order("by_contact_id");

		// List the contact's phone numbers
		phone_number.begin_filter(db::DB_SEEK_EQUAL);
		phone_number["contact_id"] = id;
		phone_number.apply_filters();
        for (phone_number.seek_first(); !phone_number.is_eof(); phone_number.seek_next()) {
			db::String number = phone_number["number"].as_string();
			PhoneNumberType type = (PhoneNumberType) (db_uint) phone_number["type"].as_int();
			int speed_dial = phone_number["speed_dial"].as_int();

			cout << "Phone number: " << number.c_str() << " (";
			switch (type) {
				case HOME:   cout << "Home"; break;
				case MOBILE: cout << "Mobile"; break;
				case WORK:   cout << "Work"; break;
				case FAX:    cout << "Fax"; break;
				case PAGER:  cout << "Pager"; break;
			}
			if (speed_dial >= 0)
				cout << ", speed dial " << speed_dial;
			cout << ")" << endl;
		}

		phone_number.close();

		cout << endl;
	}

	contact.close();
}

/**
 * Retrieve picture_name field from a contact
 */
db::String PhoneBook::get_picture_name(db_uint id)
{
	db::Table contact;
	
	contact.open(db, "contact");

	// Seek using the "$PK" index
	contact.set_sort_order("$PK");
	contact.begin_seek(db::DB_SEEK_EQUAL);
	contact["id"] = id;
	db::String picture_name;
	
	if (DB_SUCCESS(print_error(contact.apply_seek()))) {
		picture_name = contact["picture_name"].as_string();
	} else {
		cerr << "Could not find contact with id " << (long) id << endl;
	}

	contact.close();
	return picture_name;
}

/**
 * Export picture file to disk
 *
 * Demonstrates:
 * - reading the contents of a BLOB
 */
void PhoneBook::export_picture(db_uint id, const char *file_name)
{
	db::Table contact;
	
	contact.open(db, "contact");

	// Seek using the "$PK" index
	contact.set_sort_order("$PK");
	contact.begin_filter(db::DB_SEEK_EQUAL);
	contact["id"] = id;
    contact.apply_filters();
	if (DB_SUCCESS(print_error(contact.seek_first()))) {

		// Open file
		FILE *picture_file;
		if ((picture_file = fopen(file_name, "wb")) != NULL) {

			// Prepare BLOB variables
			int offset, bytes_read;
			int picture_field = contact.find_field("picture");
			db_len_t blob_size = contact.get_blob_size(picture_field);
			const db_len_t data_size = 256;
			char data[data_size];

			// Export file from BLOB to disk
			for(offset = 0; offset < blob_size; offset += data_size)
			{
				DB_SUCCESS(bytes_read = contact.read_blob(picture_field, offset, data, data_size));
				fwrite(data, bytes_read, 1, picture_file);
			}

			fclose(picture_file);

		} else {
			cerr << "Cannot open " << file_name << endl;
		}

	} else {
		cerr << "Could not find contact with id " << (long) id << endl;
	}

	contact.close();
}

/**
 * Start transaction
 */
void PhoneBook::tx_start()
{
	db.tx_begin();
}

/**
 * Commit transaction
 */
void PhoneBook::tx_commit()
{
	if (DB_FAILED(db.tx_commit())) {
		cerr << "Failed to commit transaction." << endl;
		return;
	}
}

