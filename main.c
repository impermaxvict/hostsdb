#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <sqlite3.h>

#define DATABASE_PATH "./hosts.sqlite3"

#define CREATE_STMT "CREATE TABLE IF NOT EXISTS `rules` (" \
	"`ip_address` TEXT NOT NULL, " \
	"`host_name` TEXT NOT NULL, " \
	"PRIMARY KEY(`ip_address`, `host_name`)" \
	");"

sqlite3 *db;

#define INSERT_RULE_STMT "INSERT OR IGNORE INTO rules (ip_address, host_name) VALUES (?, ?)"

#define MAX_IP_ADDRESS_LENGTH 64
#define MAX_HOST_NAME_LENGTH 1024

int parse_hosts_file(const char *path) {
	FILE *fp = fopen(path, "rb");
	if (fp == NULL) return 1;

	sqlite3_stmt *stmt;

	int rc = sqlite3_prepare_v2(db, INSERT_RULE_STMT, -1, &stmt, NULL);

	if (rc == SQLITE_OK) {
		rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
	}

	if (rc != SQLITE_OK) {
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		fclose(fp);
		return 1;
	}

	char *line = NULL;
	size_t len;
	ssize_t read;

	char ip_address[MAX_IP_ADDRESS_LENGTH];
	char host_name[MAX_HOST_NAME_LENGTH];

	while ((read = getline(&line, &len, fp)) != -1) {
		// Ignore empty lines
		if (read == 0 ||
		   (read == 1 && line[0] == '\n') ||
		   (read == 1 && line[0] == '\r') ||
		   (read == 2 && line[0] == '\r' && line[1] == '\n')) {
			continue;
		}

		// Ignore commented-out lines
		if (line[0] == '#') {
			continue;
		}
		
		ssize_t l = 0, r = 0;
		
		// Remove possible comment
		char *p = strchr(line, '#');
		if (p != NULL) {
			*p = '\0';
			r = p - line - 1;
		} else {
			r = read - 1;
		}

		// Find whitespace offsets
		while (line[l] && isspace(line[l])) l++;
		while (line[r] && isspace(line[r])) r--;
		if (l > r) {
			continue;
		}

		// Get the IP address
		ssize_t n = 0;
		while (l <= r && (line[l] == '.' || line[l] == ':' || isxdigit(line[l]))) {
			ip_address[n++] = line[l++];
		}
		ip_address[n] = '\0';
		if (strlen(ip_address) == 0) {
			continue;
		}

		if (sqlite3_bind_text(stmt, 1, ip_address, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
			free(line);
			fclose(fp);
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			return 1;
		}

		// Get the host names
		while (l <= r) {
			// Skip whitespace
			while (l <= r && isspace(line[l])) l++;

			ssize_t j = 0;
			while (l <= r && !isspace(line[l])) {
				host_name[j++] = line[l++];
			}
			host_name[j] = '\0';
			if (strlen(host_name) == 0) {
				continue;
			}

			rc = sqlite3_bind_text(stmt, 2, host_name, -1, SQLITE_TRANSIENT);
			if (rc == SQLITE_OK) {
				rc = sqlite3_step(stmt);
			}
			if (rc == SQLITE_DONE) {
				rc = sqlite3_reset(stmt);
			}

			if (rc != SQLITE_OK) {
				free(line);
				fclose(fp);
				sqlite3_finalize(stmt);
				sqlite3_close(db);
				return 1;
			}
		}
	}

	free(line);

	if (fclose(fp) != 0) return 1;

	rc = sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
	sqlite3_finalize(stmt);
	
	if (rc != SQLITE_OK) {
		sqlite3_close(db);
		return 1;
	}

	return 0;
}

int main(const int argc, const char **argv) {
	char *hosts_file = NULL;

	if (argc == 2) {
		hosts_file = (char *) *(argv + 1);
	} else {
		puts("Invalid number of arguments!");
		if (argc == 1) {
			printf("Usage: %s <hosts-file>\nExample: %s /etc/hosts\n", argv[0], argv[0]);
		}
		return 1;
	}

	puts("Parsing:");
	puts(hosts_file);

	int rc = sqlite3_open(DATABASE_PATH, &db);
	if (rc != SQLITE_OK) {
		puts("Failed to open or create the database!");
		return 1;
	}

	if (sqlite3_exec(db, CREATE_STMT, NULL, NULL, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		puts("Failed to create the rules table!");
		return 1;
	}

	if (parse_hosts_file(hosts_file) != 0) {
		sqlite3_close(db);
		puts("Failed to parse the hosts file!");
		return 1;
	}

	if(sqlite3_close(db) != SQLITE_OK) {
		puts("Failed to close the database!");
		return 1;
	}

	puts("Sucessfully parsed the hosts file!");

	return 0;
}

