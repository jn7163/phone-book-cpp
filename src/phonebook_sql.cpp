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

/** @file phonebook.cpp
 *
 * Command line example program demonstrating the ITTIA DB C++ SQL API
 */

#include "phonebook.h"
#include "dbs_error_info.h"

#include <stdio.h>

#ifdef _MSC_VER
#pragma warning (push, 1)
#endif

#include <iostream>

#ifdef __embedded_cplusplus
#define cerr cout
#else
using std::cerr;
using std::endl;
using std::cout;
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

using namespace db;

#define MAX_CONTACT_NAME        50   // Unicode characters
#define MAX_FILE_NAME           50   // ANSI characters
#define DATA_SIZE               1024 // BLOB chunk size
#define MAX_PHONE_NUMBER        20   // phone number length


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
 * Helper function to print error messages.
 */
int print_error(int rc, const Query& query)
{
    if (DB_FAILED(rc)) {
        dbs_error_info_t info = dbs_get_error_info( rc );
        String query_message = query.get_error_message();

        cerr << "ERROR " << info.name << ": " << info.description << endl;
        if (query_message.size() > 0) {
            cerr << query_message.c_str() << endl;
        }
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
    int     rc;
    char    buffer[256];
    Query q;

    //-------------------------------------------------------------------
    // Create the CONTACT table
    //   uint64         id
    //   nvarchar       name(MAX_CONTACT_NAME)
    //   uint64         ring_id
    //   varchar        picture_name(FILENAME_MAX)
    //   blob           picture
    //-------------------------------------------------------------------
    if (with_picture)
        sprintf(buffer, 
            "create table contact ("
            "  id uint64 not null,"
            "  name utf16str(%d) not null,"
            "  ring_id uint64,"
            "  picture_name varchar(%d),"
            "  picture blob,"
            "  constraint by_id primary key (id)"
            ")",
            MAX_CONTACT_NAME, MAX_FILE_NAME);
    else
        sprintf(buffer, 
            "create table contact ("
            "  id uint64 not null,"
            "  name utf16str(%d) not null,"
            "  ring_id uint64,"
            "  picture_name varchar(%d),"
            "  constraint by_id primary key (id)"
            ")",
            MAX_CONTACT_NAME, MAX_FILE_NAME);

    if  (DB_SUCCESS(rc = q.exec_direct(db, buffer))) {
        //-----------------------------------------------------------
        // Create name index on CONTACT table
        //-----------------------------------------------------------
        rc = q.exec_direct(db, "create index by_name on contact(name)");
    }

    return print_error(rc, q);
}

/**
 * Create the table "phone_number", which lists all known telephone numbers.
 */
int PhoneBook::create_table_phone_number()
{
    int     rc;
    char    buffer[256];
    Query q;

    //-------------------------------------------------------------------
    // Create the phone_number table
    //   uint64         id
    //   nvarchar       name(MAX_CONTACT_NAME)
    //   uint64         ring_id
    //   varchar        picture_name(FILENAME_MAX)
    //   blob           picture
    //-------------------------------------------------------------------
    sprintf( buffer,
        "create table phone_number ("
        "  contact_id uint64 not null,"
        "  number ansistr(%d) not null,"
        "  type uint64 not null,"
        "  speed_dial sint64,"
        "  constraint contact_ref foreign key (contact_id) references contact(id)"
        ")",
        MAX_PHONE_NUMBER);

    if  (DB_SUCCESS(rc = q.exec_direct(db, buffer))) {
        //---------------------------------------------------------------
        // Create contact_id index on PHONE_NUMBER table
        //---------------------------------------------------------------
        rc = q.exec_direct(db,
            "create index by_contact_id on phone_number(contact_id)" );
    }

    return print_error(rc, q);
}

/**
 * Create sequences. Sequences are used to generate unique identifiers.
 *
 * Demonstrates:
 * - defining sequences
 */
int PhoneBook::create_sequences()
{
    Query q;
    //-------------------------------------------------------------------
    // Create CONTACT_ID sequence
    //-------------------------------------------------------------------
    return print_error(q.exec_direct(db, "create sequence contact_id start with 1"), q);
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
    int rc = DB_NOERROR;
    StorageMode mode;        // Default database storage mode options
    mode.file_mode = file_mode;

    rc = db.open(database_name, mode);

    if (DB_FAILED(rc)) {
        cerr << "Unable to open database: [" << database_name << "]." << endl;
        print_error(rc);
    }

    return rc;
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
    StorageMode mode;
    mode.file_mode = file_mode;
    if (file_mode == db::DB_MEMORY_STORAGE) {
        mode.memory_storage_size = MEMORY_STORAGE_SIZE;
        cout << "Creating " << mode.memory_storage_size << " byte memory storage." << endl;
    }

    //-------------------------------------------------------------------
    // Create a new empty database, overwriting existing files
    //-------------------------------------------------------------------
    if  (DB_FAILED( rc = db.create(database_name, mode) )) {
        cerr << "Error creating new database: [" << database_name << "]." << endl;
        print_error(rc);
        return rc;
    }
    if (DB_FAILED( rc = create_tables(file_mode != db::DB_MEMORY_STORAGE) )) {
        cerr << "Error creating tables" << rc << endl;
        return rc;
    }
    if (DB_FAILED( rc = create_sequences() )) {
        cerr << "Error creating sequences" << rc << endl;
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
    Query       q;
    Sequence    id_sequence;
    db_uint         id;

    id_sequence.open(db, "contact_id");
    id_sequence.get_next_value(id);

    q.prepare(db,
        "insert into contact (id, name, ring_id, picture_name) "
        "  values ($<integer>0, $<nvarchar>1, $<integer>2, $<varchar>3) ");
    q.param(0) = id;
    q.param(1) = name;
    q.param(2) = ring_id;
    q.param(3) = picture_name;
    if  (DB_SUCCESS(print_error(q.execute(), q))) {
        //---------------------------------------------------------------
        // Insert the BLOB field
        //---------------------------------------------------------------
        update_contact_picture(id, picture_name);
    } else {
        //---------------------------------------------------------------
        // Error returned from execute
        //---------------------------------------------------------------
        id = 0;
    }

    return id;
}

/**
 * Update the value of a BLOB field.
 * Because BLOB fields can be larger than available memory,
 * they are accessed through a streaming interface instead of SQL.
 */
void PhoneBook::update_contact_picture(db_uint contact_id, const char *picture_name)
{
    Table contact;

    contact.open(db, "contact");

    //---------------------------------------------------------------
    contact.set_sort_order("$PK");
    contact.begin_seek(DB_SEEK_EQUAL);
    //---------------------------------------------------------------

    contact["id"] = contact_id;
    if (DB_SUCCESS(contact.apply_seek())) {
        //-----------------------------------------------------------
        // Open picture file
        //-----------------------------------------------------------
        FILE *picture_file;
        if ((picture_file = fopen(picture_name, "rb")) != NULL) {

            //-------------------------------------------------------
            // Prepare BLOB variables
            //-------------------------------------------------------
            int picture_field = contact.find_field("picture");
            int num_chunks = 0;
            int bytes_read = 0;
            const db_len_t data_size = 256;
            char data[data_size];

            //-------------------------------------------------------
            // Store picture into BLOB field
            //-------------------------------------------------------
            while((bytes_read = (int)fread(data, 1, data_size, picture_file)) > 0) {
                contact.write_blob(picture_field, data_size * num_chunks, data, bytes_read);
                num_chunks++;
            }
            fclose(picture_file);

        } else {
            cerr << "Cannot open " << picture_name << endl;
        }
    } else {
        cerr << "Could not find contact with id " << (int) contact_id << endl;
    }

    contact.close();
}

/**
 * Insert a phone entry into the database.
 */
void PhoneBook::insert_phone_number(db_uint contact_id, const char *number, PhoneNumberType type, db_sint speed_dial)
{
    Query q;

    q.prepare(db,
        "insert into phone_number (contact_id,number,type,speed_dial) "
        "  values ($<integer>0, $<varchar>1, $<integer>2, $<integer>3) ");

    q.param(0) = contact_id;
    q.param(1) = number;
    q.param(2) = (int) type;
    q.param(3) = speed_dial;

    print_error(q.execute(), q);
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
    Query q;

    q.prepare(db,
        "update contact "
        "  set name = $<nvarchar>1 "
        "  where id = $<integer>0 ");

    q.param(0) = id;
    q.param(1) = newname;

    print_error(q.execute(), q);
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
    Query q;

    //---------------------------------------------------------------
    // Remove the corresponding recs from the phone_number table.
    //---------------------------------------------------------------
    q.prepare(db,
        "delete from phone_number "
        "  where contact_id = $<integer>0 ");
    q.param(0) = id;

    if (DB_SUCCESS(print_error(q.execute(), q))) {
        //-------------------------------------------------------------------
        // Remove record from contact table.
        //-------------------------------------------------------------------
        q.prepare(db,
            "delete from contact "
            "  where id = $<integer>0 ");
        q.param(0) = id;

        print_error(q.execute(), q);
    }
}

/**
 * Briefly list all contacts in the database.
 */
void PhoneBook::list_contacts_brief()
{
    Query       q;
    const char  *cmd;

    cmd = "select id, name "
          "  from contact "
          "  order by name ";

    if  (DB_SUCCESS(print_error(q.exec_direct(db, cmd), q))) {
        //---------------------------------------------------------------
        // Bind local data fields to the data retrieved by the SQL call.
        // The field number is determined by the order of the fields
        // in the select statement.
        //
        // Field bindings can be created before or after the query is executed.
        //---------------------------------------------------------------
        IntegerField  id(q, "id");
        WStringField  name(q, "name");

        for (q.seek_first(); !q.is_eof(); q.seek_next()) {
            char name_mbs[50];
            wcstombs(name_mbs, WString(name).c_str(), sizeof name_mbs/sizeof name_mbs[0]);

            cout << (long) id << '\t';
            cout << name_mbs << endl;
        }
    }
    return;
}

/**
 * List all contacts in the database with full phone numbers
 *
 * Demonstrates:
 * - parent/child relationships
 */
void PhoneBook::list_contacts(int sort)
{
    Query       q;
    const char  *cmd;
    uint64_t    prev_id = 0;

    const char* query_by_name = 
        "select A.id, A.name, A.ring_id, A.picture_name, B.number, B.type, B.speed_dial"
        "  from contact A, phone_number B"
        "  where A.id = B.contact_id"
        "  order by A.name, B.type";
    const char* query_by_id =
        "select A.id, A.name, A.ring_id, A.picture_name, B.number, B.type, B.speed_dial"
        "  from contact A, phone_number B"
        "  where A.id = B.contact_id"
        "  order by A.id, B.type";
    const char* query_by_ring_id_name =
        "select A.id, A.name, A.ring_id, A.picture_name, B.number, B.type, B.speed_dial"
        "  from contact A, phone_number B"
        "  where A.id = B.contact_id"
        "  order by A.ring_id, A.name, B.type";

    /* Choose the query for the selected sort order. */
    switch (sort) {
        case 0:
            cmd = query_by_id;
            break;
        case 1:
            cmd = query_by_name;
            break;
        case 2:
            cmd = query_by_ring_id_name;
            break;
        default:
            return;
    }

    if  (DB_SUCCESS(print_error(q.exec_direct(db, cmd), q))) {
        //---------------------------------------------------------------
        // Bind local data fields to the data retrieved by the SQL call.
        // The field number is determined by the order of the fields
        // in the select statement.
        //
        // Field bindings can be created before or after the query is executed.
        //---------------------------------------------------------------
        IntegerField    id          (q, "id");
        WStringField    name        (q, "name");
        IntegerField    ring_id     (q, "ring_id");
        StringField     picture_name(q, "picture_name");
        StringField     number      (q, "number");
        IntegerField    type        (q, "type");
        IntegerField    speed_dial  (q, "speed_dial");

        for (q.seek_first(); !q.is_eof(); q.seek_next()) {
            //-----------------------------------------------------------
            // Otherwise, if brief is NOT set we want to display the
            // entire row from the selected data.
            //
            // For contacts with numerous phone numbers, only display
            //   the ID, NAME, RING_TONE, and PICTURE_NAME once.
            //-----------------------------------------------------------
            if  ( (uint64_t)id != prev_id ) {
                char name_mbs[50];
                wcstombs(name_mbs, WString(name).c_str(), sizeof name_mbs/sizeof name_mbs[0]);
                
                prev_id = id;
                cout << "Id: " << (long) id << endl;
                cout << "Name: " << name_mbs << endl;

                if (!ring_id.is_null())
                    cout << "Ring tone id: " << (int) ring_id << endl;

                if (!picture_name.is_null())
                    cout << "Picture name: " << String(picture_name).c_str() << endl;
            }

            cout << "Phone number: " << String(number).c_str() << " (";
            switch ((long)type) {
                case HOME:   cout << "Home"; break;
                case MOBILE: cout << "Mobile"; break;
                case WORK:   cout << "Work"; break;
                case FAX:    cout << "Fax"; break;
                case PAGER:  cout << "Pager"; break;
            }
            if (((long)speed_dial) >= 0)
                cout << ", speed dial " << (long) speed_dial;

            cout << ")" << endl;
        }
    }
    return;
}

/**
 * Retrieve picture_name field from a contact
 */
String PhoneBook::get_picture_name(db_uint id)
{
    Query q;

    //-------------------------------------------------------------------
    // Select a specific record from the contact table.
    //-------------------------------------------------------------------
    print_error(q.prepare(db, "select picture_name from contact where id = $<integer>0"), q);
    q.param(0) = id;

    if  (DB_SUCCESS(print_error(q.execute(), q))) {
        q.seek_first();
        return String(q[0].as_string());
    }

    return String("");
}

/**
 * Export picture file to disk
 *
 * Demonstrates:
 * - reading the contents of a BLOB
 */
void PhoneBook::export_picture(db_uint id, const char *file_name)
{
    Query q;
    BlobField   blob;

    enum FieldOrder {
        PICTURE_FIELD = 0
    };

    //-------------------------------------------------------------------
    // Select a specific record from the contact table.
    //-------------------------------------------------------------------
    print_error(q.prepare(db, "select picture from contact where id = $<integer>0"), q);
    q.param(0) = id;

    if  (DB_SUCCESS(print_error(q.execute(), q))) {

        blob.attach(q, PICTURE_FIELD);

        //---------------------------------------------------------------
        // Position the cursor to the first record (only 1 record).
        //---------------------------------------------------------------
        if  (q.seek_first() == DB_NOERROR) {
            int             offset, bytes_read;
            db_len_t        blob_size = blob.size();
            const db_len_t  data_size = 256;
            char            data[data_size];

            //-----------------------------------------------------------
            // Open the output file.
            //-----------------------------------------------------------
            FILE *picture_file;
            if ((picture_file = fopen(file_name, "wb")) != NULL) {

                //-------------------------------------------------------
                // Export the BLOB to the output image file
                //-------------------------------------------------------
                for(offset = 0; offset < blob_size; offset += data_size) {
                    if  (bytes_read = blob.read(offset, data, data_size)) {
                        fwrite(data, bytes_read, 1, picture_file);
                    }
                }

                //-------------------------------------------------------
                // Close the output file.
                //-------------------------------------------------------
                fclose(picture_file);

            } else {
                cerr << "Cannot open " << file_name << endl;
            }

        } else {
            cerr << "Could not find contact with id " << (long) id << endl;
        }
    }
    return;
}

/**
 * Start transaction
 */
void PhoneBook::tx_start()
{
    Query q;
    // Equivalent to: db.tx_begin();
    print_error(q.exec_direct(db, "start transaction"), q);
}

/**
 * Commit transaction
 */
void PhoneBook::tx_commit()
{
    Query q;
    // Equivalent to: db.tx_commit();
    print_error(q.exec_direct(db, "commit"), q);
}

