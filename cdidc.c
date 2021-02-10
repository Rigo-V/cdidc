/* cdidc - calculates MusicBrainz disc id of a CD using libdiscid
 * Copyright © 2021 Riku Viitanen <riku.viitanen@protonmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)
#define N_(str) ngettext(str)

#include <discid/discid.h>


/* Command to open user's default browser */
#define SYSTEM_BROWSER_CMD "xdg-open"

/* This message is printed when 'cdidc -v' is run */
static const char *copyright_message = "Copyright © 2021 Riku Viitanen\n\
License GPLv3+: GNU GPL version 3 or later\
 <https://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.";


/* Launches user's preferred browser as a child
 * process and returns immediately after forking. */
static void launch_browser(char *browser, char *url) {
	pid_t cpid = 0;
	cpid = fork();

	if (cpid == 0) {
		/* I am the child */

		/* Clone stdout and stderr file descriptors for later use */
		int stdout_copy = dup(1);
		int stderr_copy = dup(2);

		/* Close stdout and stderr */
		close(1);
		close(2);

		/* Since open() always assigns the lowest available fd,
		 * these will make stdout and stderr point at /dev/null */
		open("/dev/null", O_WRONLY);
		open("/dev/null", O_RDONLY);

		/* Arguments passed to the browser opener */
		char *argv_launch[] = {browser, url, NULL};

		/* If this succeeds, it will replace the child process
		 * and none of the lines coming after will actually execute. */
		execvp(browser, argv_launch);

		dprintf(stderr_copy, _("%s: Failed to start %s: %s\n"),
	                NAME, browser, strerror(errno));


		dprintf(stdout_copy, _("Submission URL: %s\n"), url);

		/* Cleanup */
		close(1);
		close(2);
		close(stdout_copy);
		close(stderr_copy);
	}
}


static void print_version() {
	printf("%s %s\n", NAME, VERSION);
}


static void print_usage(char *program_name) {
	printf(_("Usage: %s [OPTIONS]\n"), program_name);
	puts(_("Calculate MusicBrainz or CDDB IDs of a compact disc."));
	puts(_("If neither type of ID is specified, both are printed.\n"));

	printf(_("-d DEVICE   Optical disc drive to use. Defaults to %s\n"),
	       discid_get_default_device());
	puts(_("-c          Print CDDB ID of CD"));
	puts(_("-m          Print MusicBrainz Disc ID of CD"));
	puts(_("-s          Submit Disc ID to MusicBrainz using the default browser"));
	puts(_("-w BROWSER  Use BROWSER instead of the system default (implies -s)"));
	puts(_("-b          Format the output more briefly"));
	printf(_("-v          Print version (%s) information and exit\n"),
	                                                        VERSION);
	puts(_("-h          Print this help message and exit"));
}


static void get_options(int argc, char **argv, bool *flag_brief, char **device,
                        bool *flag_submit_id, bool *flag_calculate_mb_id,
		        bool *flag_calculate_cddb_id, char **browser) {
	
	int o;
	bool flag_cddb = false;
	bool flag_mb = false;

	/* Parse options */
	while (true) {
		o = getopt(argc, argv, "d:cmsbvhw:");

		/* Detect end of options */
		if (o == -1) break;

		switch (o) {
			case 'd':
				*device = optarg;
				break;

			case 'c':
				flag_cddb = true;
				break;

			case 'm':
				flag_mb = true;
				break;

			case 's':
				*flag_submit_id = true;
				break;

			case 'b':
				*flag_brief = true;
				break;

			case 'v':
				print_version();
				puts(copyright_message);
				exit(0);

			case 'h':
				print_usage(argv[0]);
				exit(0);

			case 'w':
				*browser = optarg;
				*flag_submit_id = true;
				break;

			default:
				print_usage(argv[0]);
				exit(-1);
		}
	}

	/* If neither of these flags is set, set both */
	*flag_calculate_mb_id = flag_mb || (!flag_cddb);
	*flag_calculate_cddb_id = flag_cddb || (!flag_mb);
}


int main(int argc, char **argv) {

	/* Use user's preferred locale. */
	setlocale(LC_ALL, "");
	bindtextdomain(NAME, "/usr/share/locale/");
	textdomain(NAME);

	bool flag_print_mb_id = false;
	bool flag_print_cddb_id = false;
	bool flag_brief = false;
	bool flag_submit_id = false;

	/* Let libdiscid decide the default cd drive on user's system. */
	char *device = discid_get_default_device();

	char *browser = SYSTEM_BROWSER_CMD;


	get_options(argc, argv, &flag_brief, &device, &flag_submit_id,
	                  &flag_print_mb_id, &flag_print_cddb_id, &browser);


	DiscId *disc = discid_new();

	/* Make sure disc is readable */
	if (discid_read_sparse(disc, device, 0) == 0) {
		fprintf(stderr, "libdiscid: %s\n", discid_get_error_msg(disc));
		discid_free(disc);

		return 1;
	}

	if (flag_print_mb_id || flag_submit_id) {
		char *mbid = discid_get_id(disc);

		if (flag_print_mb_id) {
			if (flag_brief)
				puts(mbid);
			else
				printf("Musicbrainz Disc ID: %s\n", mbid);
		}

		if (flag_submit_id) {
			char *url = discid_get_submission_url(disc);
			launch_browser(browser, url);
		}
	}

	if (flag_print_cddb_id) {
		if (flag_brief)
			puts(discid_get_freedb_id(disc));
		else
			printf("CDDB ID: %s\n", discid_get_freedb_id(disc));
	}


	/* Cleanup */
	discid_free(disc);

	return 0;
}

