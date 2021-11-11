#include<fstream>
#include<iostream>
#include<string>
#include<libssh/sftp.h>
#include<stdio.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sqlite3.h>

using namespace std;

int sftp_con(ssh_session my_session, sftp_session my_sftp, const char* sftp_path, const char* local_path)
{
	int s_init;
	int mode = S_IEXEC | S_IREAD | S_IWRITE;
	int type = O_WRONLY | O_CREAT | O_TRUNC;
	const char* hello = local_path;
	int buf_size = strlen(hello);
	int write_n_bytes;
	int file_close;

	sftp_file file;

	my_sftp = sftp_new(my_session);
	if (my_sftp == NULL)
	{
		cerr << "Error!\n";
		ssh_get_error(my_session);
		return SSH_ERROR;
	}

	s_init = sftp_init(my_sftp);
	if (s_init != NULL)
	{
		cerr << "Error!\n";
		sftp_get_error(my_sftp);
		return SSH_ERROR;
	}

	file = sftp_open(my_sftp, sftp_path, type, mode);
	if (file == NULL)
	{
		cerr << "Error!\n";
		ssh_get_error(my_session);
		return SSH_ERROR;
	}

	write_n_bytes = sftp_write(file, hello, buf_size);
	if (write_n_bytes != buf_size)
	{
		cerr << "Error!\n";
		ssh_get_error(my_session);
		return SSH_ERROR;
	}

	file_close = sftp_close(file);
	if (file_close == SSH_ERROR)
	{
		cerr << "Error!\n";
		ssh_get_error(my_session);
		return file_close;
	}

	return SSH_OK;
}

int callback(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int main(int argc, const char* argv[])
{
	fstream the_file("file.txt", ios::in);
	if (!the_file.is_open())
	{
		cout << "Could not open file\n";
	}

	const int arr_size = 100;

	char sftp_host[arr_size];
	the_file.getline(sftp_host, arr_size);

	char sftp_port[arr_size];
	the_file.getline(sftp_port, arr_size);

	char sftp_user[arr_size];
	the_file.getline(sftp_user, arr_size);

	char sftp_password[arr_size];
	the_file.getline(sftp_password, arr_size);

	char sftp_remote_dir[arr_size];
	the_file.getline(sftp_remote_dir, arr_size);

	char local_dir[arr_size];
	the_file.getline(local_dir, arr_size);

	char sql_user[arr_size];
	the_file.getline(sql_user, arr_size);

	char sql_password[arr_size];
	the_file.getline(sql_password, arr_size);

	char sql_database[arr_size];
	the_file.getline(sql_database, arr_size);

	///////////////////////////////////////////////////////////////
	int con;
	int ver = SSH_LOG_PROTOCOL;
	int port = 22;

	ssh_session session = ssh_new();
	sftp_session sftp = sftp_new(session);

	if (session == NULL)
	{
		cerr << "Error session!\n";
		exit(-1);
	}

	ssh_options_set(session, SSH_OPTIONS_HOST, &sftp_host);
	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &ver);
	ssh_options_set(session, SSH_OPTIONS_PORT, &port);
	ssh_options_set(session, SSH_OPTIONS_USER, &sftp_user);

	con = ssh_connect(session);
	if (con != SSH_OK)
	{
		cerr << "Error connect!\n";
		ssh_get_error(session);
		exit(-1);
	}

	sftp_con(session, sftp, sftp_remote_dir, local_dir);

	ssh_disconnect(session);
	sftp_free(sftp);
	ssh_free(session);


	////////////////////////////////////////////////////////
	sqlite3* db;
	char* zErrMsg = 0;
	int s_open, s_exec;
	const char* sql;
	const char* data = "Callback function called";

	s_open = sqlite3_open(sql_database, &db);
	if (s_open)
	{
		cerr << "Can't open db\n";
		sqlite3_errmsg(db);
	}
	else
	{
		cerr << "Opened db\n";
	}

	sql = "CREATE TABLE ("  \
		"NAME           TEXT		 NOT NULL," \
		"PASSWORD       CHAR(50)     NOT NULL);";

	s_exec = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (s_exec != SQLITE_OK)
	{
		cerr << "SQL error!\n";
		sqlite3_free(zErrMsg);
	}
	else
	{
		cerr << "Table created! \n";
	}
	sqlite3_close(db);
	return 0;
}