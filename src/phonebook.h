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

#ifndef PHONEBOOK_H
#define PHONEBOOK_H 1

#include <ittia/db++.h>


/* Use a local database file. */
#define DATABASE_NAME_LOCAL     "phone_book.db"
/* Use the IPC client protocol to access database through dbserver. */
#define DATABASE_NAME_SERVER    "idb+tcp://localhost/phone_book.db"

/* Use 128KiB of RAM for memory storage, when selected. */
#define MEMORY_STORAGE_SIZE     128 * 1024

/** 
 * A list of telephone contacts stored on a mobile phone
 */
class PhoneBook {
private:
	db::Database db;

public:

	/** 
	 * Types of telephone numbers
	 */
	enum PhoneNumberType {
		HOME = 0,
		MOBILE,
		WORK,
		FAX,
		PAGER
	};

	/**
	 * Types of phone call events
	 */
	enum CallLogType {
		SENT,
		RECEIVED,
		MISSED
	};

private:

	int create_tables(bool with_picture);
	int create_table_contact(bool with_picture);
	int create_table_phone_number();
	int create_sequences();

public:

	int open_database(int file_mode, const char* database_name);
	int create_database(int file_mode, const char* database_name);
	int close_database();

	db_uint insert_contact(const wchar_t *name, db_uint ring_id, const char *picture_name);
	void insert_phone_number(db_uint contact_id, const char *number, PhoneNumberType type, db_sint speed_dial);

	void update_contact_name(db_uint id, const wchar_t *newname);
    void update_contact_picture(db_uint contact_id, const char *picture_name);
	void remove_contact(db_uint id);

	void list_contacts_brief();
	void list_contacts(int sort);

	db::String get_picture_name(db_uint id);
	void export_picture(db_uint id, const char *file_name);

	void tx_start();
	void tx_commit();
};


#endif

