#include <iostream>
#include <fstream>
/* 
If include <mysql.h> returns error, perhaps you must set include folder and lib folder 
from MySql Libraries folder which is placed in main folder of  the project.
1. Right click on 'Garmory' in solution explorer -> properties -> C/C++ -> set 'Include' folder 
from MySql Libraries folder to Additional Include Directories.
2. Then go for Linker tab and set lib folder from MySql Libraries to Additional Libraries Directories.
3. Last go for Input tab in Linker and add 'libmysql.lib' to Additional Dependencies.
*/
#include <mysql.h>
#include <string>
#include <cstdlib>

/* 
Function cleanTable truncates MySql table numbers at program start in order to maintain ID order.
Function cleanTable is must for program to work properly and must be use before putting new
values into database.
*/
void cleanTable(MYSQL* connection) {
	std::string deleteSql = "TRUNCATE TABLE numbers";

	if (!mysql_query(connection, deleteSql.c_str())) {
		std::cout << "Przygotowanie bazy danych powiodlo sie." << std::endl;
	}
	else {
		std::cout << "Nie udalo sie przygotowac bazy danych przed dzialaniem programu: " << mysql_error(connection) << std::endl;
	}
}

/* 
Function fillTable fills table numbers in accordance with the program assumptions.
Puts in the database in the table 'numbers' to the field 'value' numeric values ​​from 1 to 1000 except:
* numbers divisible by 4, where the field 'value' has the value 'Garmory',
* numbers divisible by 7, where the field 'value' has the value 'Praca',
* numbers divisible by 4 and 7 simultaneously, where the field 'value' has the value 'GarmoryPraca'.
*/
void fillTable(MYSQL* connection) {
	/* Mutable query string */
	std::string sql = "INSERT INTO `numbers` (`id`, `value`) VALUES (NULL, '1')";

	/* 
	Instead of querying MySQL Server like: 
	1. "INSERT INTO 'numbers' ('id', 'value') VALUES (NULL, '1')",
	2. "INSERT INTO 'numbers' ('id', 'value') VALUES (NULL, '2')" ... etc.
	We create one query mutable string and append to it. Then we query once with long string like:
	"INSERT INTO 'numbers' ('id', 'value') VALUES (NULL, '1')", (NULL, '2')", (NULL, '3') , (NULL, 'Garmory')" ... etc.
	Also we start i = 2 becouse we put first value already in mutable query string above.
	*/
	for (int i = 2; i < 1001; i++) {
		if (i % 4 == 0 && i % 7 == 0) {
			sql.append(", (NULL, 'GarmoryPraca')");
		}
		else if (i % 4 == 0) {
			sql.append(", (NULL, 'Garmory')");
		}
		else if (i % 7 == 0) {
			sql.append(", (NULL, 'Praca')");
		}
		else {
			sql.append(", (NULL, '" + std::to_string(i) + "')");
		}
	}

	if (!mysql_query(connection, sql.c_str())) {
		std::cout << "Program wykonal swoje zadanie." << std::endl;
	}
	else {
		std::cout << "Nastapil nastepujacy blad programu podczas wprowadzania danych do bazy danych: " << mysql_error(connection) << std::endl;
	}
}

/* 
Function readConfig gets data from 'config.dat' file placed in the program main directory.
It works on the original variables, not on copies.
*/
void readConfig(std::string *db_host, std::string *db_user, std::string *db_pass, std::string *db_name) {
	std::fstream file;
	file.open("config.dat", std::ios::in);

	if (file.good()) {
		std::string line;
		int line_nr = 1;
		unsigned __int64 pos;

		/* 1. We loop every line in the config.dat file. */
		while (getline(file, line)) {

			/* 
			2. And when we find '=' sign, we get all line after.
			Then we look same way for next line until all crucial info is obtained.
			*/
			pos = line.find("=");

			switch (line_nr) {
				/* Variable 'pos + 1' becouse we don't obtain '=' itself. */
				case 1: *db_host = line.substr((unsigned __int64)pos + 1); break;
				case 2:	*db_user = line.substr((unsigned __int64)pos + 1); break;
				case 3:	*db_pass = line.substr((unsigned __int64)pos + 1); break;
				case 4: *db_name = line.substr((unsigned __int64)pos + 1); break;
				default: break;
			}

			line_nr++;
		}

		std::cout << "Udalo sie pobrac dane z pliku config.dat poprawnie." << std::endl;
		file.close();
	}
	else {
		
		/* 
		If we can't open it, we have to make sure that config.dat is in the main folder (Source Files).
		It can't be anywhere else unless we change path to it.
		*/
		std::cout << "Nie udalo sie otworzyc pliku config.dat z danymi potrzebnymi do polaczenia z baza danych." << std::endl;
		exit(0);
	}
}

/* 
Function corectness is a test function in order to check if values got written correctly in database.
Returns true if program runs correctly and false if corrupted.
Legal input: MySQL* and choose one of "Garmory", "Praca", "GarmoryPraca" to check its id value.
*/
bool corectness(MYSQL* connection, std::string value) {
	MYSQL_ROW row;
	MYSQL_RES* res;

	std::string sql;
	
	/* 
	We select what value we want to query and test its 'id'.
	For instance let's check if all 'Garmory' values are correctly pushed to database.
	Let's query first condition.
	*/
	if (value == "Garmory") {
		sql = "SELECT id FROM numbers WHERE value = 'Garmory'";
	}
	else if (value == "Praca") {
		sql = "SELECT id FROM numbers WHERE value = 'Praca'";
	}
	else if (value == "GarmoryPraca") {
		sql = "SELECT id FROM numbers WHERE value = 'GarmoryPraca'";
	}
	else {
		std::cout << "Function corectness did not recognize entered parameters." << std::endl;
		std::cout << "Provide MYSQL* connection and 'Gramory' or 'Praca' or 'GarmoryPraca'." << std::endl;
		return false;
	}

	/* 
	Next we connect to database and query our sql. We supposed that we're looking for 'Garmory' value.
	So let's get it.
	*/
	if (!mysql_query(connection, sql.c_str())) {
		res = mysql_store_result(connection);
		
		bool failure = false;
		while (row = mysql_fetch_row(res)) {
			//std::cout << row[0] << " | " << row[1] << std::endl;

			/* 
			We are here checking in every row if id of 'Garmory' value in database is divisible by 4.
			If it is then we have failure = false, becouse it's not failure.
			But if row contains id which is not divisible by 4, then we change failure = true
			becouse in our assumption 'Garmory' can be only paired with divisible by 4 number.
			We check failure variable every loop and if it occurs we handle it later.
			*/
			if (value == "Garmory") {
				if (std::atoi(row[0]) % 4 == 0 && std::atoi(row[0]) % 7 != 0) {
					failure = false;
				}
				else {
					failure = true;
				}
			}

			/* 
			Same logic for others conditions.
			*/
			else if (value == "Praca") {
				if (std::atoi(row[0]) % 7 == 0 && std::atoi(row[0]) % 4 != 0) {
					failure = false;
				}
				else {
					failure = true;
				}
			}
			else if (value == "GarmoryPraca") {
				if (std::atoi(row[0]) % 4 == 0 && std::atoi(row[0]) % 7 == 0) {
					failure = false;
				}
				else {
					failure = true;
				}
			}
			else {
				std::cout << "Nieoczekiwany blad testu." << std::endl;
			}

			/* 
			This is the moment we handle our variable (if occured). We message that program didn't passed
			the test and we message where's the error.
			Then we quit loop and whole function returns false (it means that program failed).
			*/
			if (failure == true) {
				std::cout << "Program nie przeszedl testu pomyslnie." << std::endl;
				std::cout << "Bledny rekord dla id = "
					+ std::to_string(atoi(row[0])) + "." << std::endl;
				return false;
				break;
			}
		}
		
		/* 
		If any conditions above didn't change failure to true then it means that corectness function
		hasn't found anything incorrect in our program and assumptions are fulfilled.
		*/
		return true;
	}
	else {
		std::cout << "Test zostal blednie napisany lub nie mozna sie polaczyc z baza danych." << mysql_error << std::endl;
	}
}

int main()
{
	MYSQL* conn;
	conn = mysql_init(0);

	/* 
	We declare variables in order to pass it to connection function. Those variables are our authenticators.
	We declare it here and pass it to readConfig function mentioned above.
	Function readConfig work on original variables so readConfig overwrite values of it.
	When values are overwritten we pass it to mysql_real_connect function.
	*/
	std::string db_host, db_user, db_pass, db_name;
	readConfig(&db_host, &db_user, &db_pass, &db_name);

	if (mysql_real_connect(conn, db_host.c_str(), db_user.c_str(), db_pass.c_str(), db_name.c_str(), 0, NULL, 0)) {
		std::cout << "Udalo sie polaczyc z baza danych." << std::endl;
		//std::cout << db_host << " | " << db_user << " | " << db_pass << " | " << db_name << std::endl;

		cleanTable(conn);
		fillTable(conn);
	
		/* 
		We can check if values are pushed correctly by corectness function.
		*/
		if(corectness(conn, "Garmory"))
			std::cout << "Wartosc Garmory wprowadzona poprawnie." << std::endl;

		if (corectness(conn, "Praca"))
			std::cout << "Wartosc Praca wprowadzona poprawnie." << std::endl;

		if (corectness(conn, "GarmoryPraca"))
			std::cout << "Wartosc GarmoryPraca wprowadzona poprawnie." << std::endl;
	}
	else {
		std::cout << "Nie udalo sie poloczyc z baza danych." << std::endl;
	}

	mysql_close(conn);
	return 0;
}