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

#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning (push, 1)
#endif

#include <iostream>

#ifdef __embedded_cplusplus
#define cerr cout
#else
using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::flush;
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#define DEFAULT_PICTURE "unknown.png"


/**
 * Console application for browsing the phone book.
 */
class PhoneBookConsoleApp {
    PhoneBook pbook;

public:
    //=======================================================================
    // CONNECT TO DATABASE
    //=======================================================================
    int connect()
    {
        int     connection_method;
        const char* database_name;
        int     storage_mode;

        /* Prompt for a connection method. */
        do {
            connection_method = connection_menu();
            if (connection_method == 0)
                return 1;

            /* Check the library disposition before connecting to a server. */
            if ((connection_method == 3 || connection_method == 4) &&
                db_info(NULL, DB_INFO_DISPOSITION) == DB_DISPOSITION_STANDALONE)
            {
                connection_method = 0;
                printf("This is a stand-alone build of ITTIA DB SQL.\n");
                printf("Client/server is not supported; select another option.\n\n");
            }
        } while (connection_method < 1 || connection_method > 4);

        /* First choice is file vs. memory storage. */
        storage_mode = (connection_method - 1) & 0x1 ? db::DB_MEMORY_STORAGE : db::DB_FILE_STORAGE;
        /* Second choice is local vs. server storage. */
        database_name = (connection_method - 1) & 0x2 ? DATABASE_NAME_SERVER : DATABASE_NAME_LOCAL;

        //-------------------------------------------------------------------
        // Open or create the database
        //-------------------------------------------------------------------
        int result = pbook.open_database(storage_mode, database_name);

        /* Start server if a connection error occurs. */
        if (result == DB_ESOCKETOPEN) {
            printf("Cannot connect to server. Starting server in this process.\n");
            db_server_start(NULL);
            result = pbook.open_database(storage_mode, database_name);
        }

        if (result == DB_ENOENT) {
            // The database does not exist, so create it
            cout << "Creating new database file" << endl;

            if (DB_SUCCESS(pbook.create_database(storage_mode, database_name))) {
                cout << "Populating tables with sample data" << endl;
                populate_tables();
            }
            else {
                return 1;
            }
        }
        else if (!DB_SUCCESS(result))
        {
            return 1;
        }

        return 0;
    }

    //=======================================================================
    // CLASS DESTRUCT FUNCTION
    //=======================================================================
    ~PhoneBookConsoleApp()
    {
        pbook.close_database();
    }

    int connection_menu()
    {
        int choice = -1;
        cout << "\
------ Phone Book ------\n\
Choose a database connection method:\
\n\
1) Open file storage\n\
2) Open memory storage\n\
3) Connect to ITTIA DB Server on localhost, open file storage\n\
4) Connect to ITTIA DB Server on localhost, open memory storage\n\
0) Quit\n\
\n\
Enter the number of your choice: " << flush;

        cin >> choice;

        //-------------------------------------------------------------------
        // Ignore remaining characters on the input line
        //-------------------------------------------------------------------
        cin.ignore(1000, '\n');
        cout << endl;
        return choice;
    }

    //=======================================================================
    // TABLE SEEDING function to add default data into newly created 
    //   phone_book.db database tables.
    //=======================================================================
    void populate_tables()
    {
        pbook.tx_start();

        //-------------------------------------------------------------------
        // Insert a few contacts
        //-------------------------------------------------------------------
        pbook.insert_phone_number(pbook.insert_contact(L"Bob", 0, DEFAULT_PICTURE), "206-555-1000", PhoneBook::MOBILE, -1);
        pbook.insert_phone_number(pbook.insert_contact(L"Sue", 7, DEFAULT_PICTURE), "206-555-3890", PhoneBook::HOME, 0);

        //-------------------------------------------------------------------
        // Fred has many phone numbers
        //-------------------------------------------------------------------
        db_uint id = pbook.insert_contact(L"Fred", 7, DEFAULT_PICTURE);
        pbook.insert_phone_number(id, "206-555-1308", PhoneBook::HOME, 5);
        pbook.insert_phone_number(id, "206-555-2335", PhoneBook::MOBILE, -1);
        pbook.insert_phone_number(id, "206-555-5361", PhoneBook::PAGER, -1);

        pbook.tx_commit();
    }


    //=======================================================================
    // MAIN MENU display and input function
    //=======================================================================
    int menu()
    {
        int choice = -1;
        cout << "------ Phone Book ------\n"
                "\n"
                "1) Add contact\n"
                "2) Remove contact\n"
                "3) Add phone number to existing contact\n"
                "4) Rename contact\n"
                "5) List contacts by name\n"
                "6) List contacts by id\n"
                "7) List contacts by ring id, name\n"
                "8) Export picture from existing contact\n"
                "0) Quit\n"
                "\n"
                "Enter the number of your choice: " << flush;

        cin >> choice;

        //-------------------------------------------------------------------
        // Ignore remaining characters on the input line
        //-------------------------------------------------------------------
        cin.ignore(1000, '\n');
        cout << endl;
        return choice;
    }

    //=======================================================================
    // MAIN MENU processing loop
    //=======================================================================
    void run()
    {
        for (int choice = menu(); choice != 0; choice = menu()) {
            switch (choice) {
                case 1: // Add contact
                    add_contact();
                    break;
                case 2: // Remove contact
                    remove_contact();
                    break;
                case 3: // Add phone number to existing contact
                    add_phone_number();
                    break;
                case 4: // Rename contact
                    rename_contact();
                    break;
                case 5: // List contacts
                    cout << "------ Contacts ------" << endl;
                    pbook.tx_start();
                    pbook.list_contacts(1);
                    pbook.tx_commit();
                    break;
                case 6: // List contacts
                    cout << "------ Contacts ------" << endl;
                    pbook.tx_start();
                    pbook.list_contacts(0);
                    pbook.tx_commit();
                    break;
                case 7: // List contacts
                    cout << "------ Contacts ------" << endl;
                    pbook.tx_start();
                    pbook.list_contacts(2);
                    pbook.tx_commit();
                    break;
                case 8: // Export picture from existing contact
                    export_picture();
                    break;
                default:
                    cout << "Unknown option: " << choice << endl;
            }
        }
    }

    //=======================================================================
    // CONTACT ADDING UI
    //=======================================================================
    void add_contact()
    {
        const int buffer_size = 256;

        char picture_name[buffer_size];
        wchar_t name[buffer_size];
        char name_mbs[buffer_size];
        unsigned int ring_id = 0;
        db_uint contact_id;

        cout << "------ Add Contact ------" << endl;
        cout << "Name: ";
        cin.getline(name_mbs, buffer_size);
        mbstowcs(name, name_mbs, buffer_size);

        cout << "Ring tone id number: ";
        cin >> ring_id;
        cin.ignore(1000, '\n');

        cout << "Picture file (\"" DEFAULT_PICTURE "\"): ";
        cin.getline(picture_name, buffer_size);
        if (picture_name[0] == '\0') {
            strcpy(picture_name, DEFAULT_PICTURE);
        }

        pbook.tx_start();
        contact_id = pbook.insert_contact(name, ring_id, picture_name);
        pbook.tx_commit();

        add_phone_number(contact_id);
    }

    //=======================================================================
    // CONTACT SELECTION UI
    //=======================================================================
    db_uint select_contact()
    {
        unsigned int id;

        cout << "Id\tName" << endl
             << "--\t----" << endl;

        pbook.tx_start();
        pbook.list_contacts_brief();
        pbook.tx_commit();

        cout << "Enter id number: ";
        cin >> id;
        cin.ignore(1000, '\n');

        return id;
    }

    //=======================================================================
    // PHONE NUMBER ADDING UI
    //=======================================================================
    void add_phone_number(db_uint contact_id = 0)
    {
        const int buffer_size = 256;

        char number[buffer_size];
        int type = 0;
        int speed_dial = -1;

        cout << "------ Add Phone Number ------" << endl;
        if (!contact_id) {
            //---------------------------------------------------------------
            // No contact id was given, so ask the user for one
            //---------------------------------------------------------------
            contact_id = select_contact();
        }

        cout << "Phone number: ";
        cin.getline(number, buffer_size);

        cout << "Phone number type: \n"
                "0) Home\n"
                "1) Mobile\n"
                "2) Work\n"
                "3) Fax\n"
                "4) Pager\n"
                "Enter the number of your choice: ";
        cin >> type;
        cin.ignore(1000, '\n');

        cout << "Speed dial number (-1=none): ";
        cin >> speed_dial;
        cin.ignore(1000, '\n');

        pbook.tx_start();
        pbook.insert_phone_number(contact_id, number, (PhoneBook::PhoneNumberType) type, speed_dial);
        pbook.tx_commit();
    }

    //=======================================================================
    // CONTACT REMOVAL UI
    //=======================================================================
    void remove_contact()
    {
        db_uint id;

        cout << "------ Remove Contact ------" << endl;
        id = select_contact();
        pbook.tx_start();
        pbook.remove_contact(id);
        pbook.tx_commit();
    }

    //=======================================================================
    // CONTACT RENAMING UI
    //=======================================================================
    void rename_contact()
    {
        const int buffer_size = 256;
        wchar_t name[buffer_size];
        char name_mbs[buffer_size];

        cout << "------ Rename Contact ------" << endl;
        db_uint id = select_contact();

        cout << "New name: ";
        cin.getline(name_mbs, buffer_size);
        mbstowcs(name, name_mbs, buffer_size);

        pbook.tx_start();
        pbook.update_contact_name(id, name);
        pbook.tx_commit();
    }

    //=======================================================================
    // PICTURE (BLOB FIELD) EXPORTING UI
    //=======================================================================
    void export_picture()
    {
        db_uint id;
        const int buffer_size = 256;
        char buffer[buffer_size];

        cout << "------ Export Picture ------" << endl;
        id = select_contact();

        pbook.tx_start();
        db::String file_name = pbook.get_picture_name(id);
        pbook.tx_commit();

        cout << "Choose a filename for picture (default=\"" <<
            file_name.c_str() << "\"): ";

        cin.getline(buffer, buffer_size);
        if (buffer[0] != '\0')
            file_name = buffer;
                
        pbook.tx_start();
        pbook.export_picture(id, file_name.c_str());
        pbook.tx_commit();
    }
};

//=======================================================================
//=======================================================================
// PROGRAM ENTRY
//=======================================================================
//=======================================================================
int main()
{
    PhoneBookConsoleApp app;

    if (app.connect())
        return 1;

	app.run();
	return 0;
}
