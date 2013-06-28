/* $Id$ */
/* File: cmd4.c */

/* Purpose: Interface commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"


/* use character class titles more often when describing a character? */
#define ABUNDANT_TITLES

#if 0 /* replaced by client-side options: \
    -- + -- = nothing \
    o1 + -- = COMPACT_PLAYERLIST+COMPACT_ALT \
    -- + o2 = COMPACT_PLAYERLIST \
    o1 + o2 = ULTRA_COMPACT_PLAYERLIST */
/* use more compact @-list to get more information displayed?
   NOTE: Requires ABUNDANT_TITLES! (in do_write_others_attributes()) */
#define COMPACT_PLAYERLIST
/* if COMPACT_PLAYERLIST is enabled, this will switch to an even denser layout,
   which in exchange displays the hostnames to all players again (tradition). */
#define COMPACT_ALT

/* use ultra compact @-list that uses only 2 lines per entry.
   NOTE: COMPACT_PLAYERLIST must be disabled when using this! */
//#define ULTRA_COMPACT_PLAYERLIST

/* print compressed gender in 1st line. If disabled, gender might instead get
   printed in the 2nd line, depending on the actual display mode.
   This can be optionally added to either COMPACT_PLAYERLIST or ULTRA_COMPACT_PLAYERLIST. */
//#define COMPACT_GENDER
#endif

/* Allow to inspect light source and ammo quiver too while target wears mummy wrapping? */
#define WRAPPING_NEW

/* Indicate that a character is in/for Ironman Deep Dive Challenge
   by displaying him in grey colour in @ list (if otherwise in neutral, ie white, colour)? */
#define IDDC_CHAR_COLOUR_INDICATOR
/* Indicate that a character is in/for Ironman Deep Dive Challenge
   by displaying his position in @ list to everyone? */
//#define IDDC_CHAR_POSITION_INDICATOR


/*
 * Check the status of "artifacts"
 *
 * Should every artifact that is held by anyone be listed?  If so, should this
 * list the holder?  Doing so might induce wars to take hold of relatively
 * worthless artifacts (like the Phial), simply because there are very few
 * (three) artifact lites.  Right now, the holder isn't listed.
 *
 * Also, (for simplicity), artifacts lying in the dungeon, or artifacts that
 * are in a player's inventory but not identified are put in the list.
 *
 * Why does Ben save the list to a file and then display it?  Seems like a
 * strange way of doing things to me.  --KLJ--
 */
/* Sort the artifact list by tval/sval before displaying it?
   todo: alternate between 2 bufs instead of copying back the buf into the main
   array each time, sorta defeats the speed advantage.. or actually pre-sort in
   init*.c already instead of repeating it everytime here ^^- C. Blue */
#define ARTS_PRE_SORT
void do_cmd_check_artifacts(int Ind, int line)
{
	int i, j, k, z;
	FILE *fff;
	char file_name[MAX_PATH_LENGTH];
	char base_name[ONAME_LEN];
	bool okay[MAX_A_IDX];
#ifdef ARTS_PRE_SORT
	int n;
	int radix_idx[MAX_A_IDX], radix_key[MAX_A_IDX];
	int radix_buf[MAX_A_IDX][10], radix_buf_cnt[10], radix_buf_idx[MAX_A_IDX][10];
#endif

	player_type *q_ptr = Players[Ind];
	bool admin = is_admin(q_ptr);
	bool shown = FALSE;

	object_type forge, *o_ptr;
	player_type *p_ptr;
	artifact_type *a_ptr;
	char fmt[10];


	/* Temporary file */
	if (path_temp(file_name, MAX_PATH_LENGTH)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "wb");

	/* Scan the artifacts */
	for (k = 0; k < MAX_A_IDX; k++) {
		a_ptr = &a_info[k];

		/* little hack: create the artifact temporarily */
		z = lookup_kind(a_ptr->tval, a_ptr->sval);
		if (z) invcopy(&forge, z);
		forge.name1 = k;

		/* Default */
		okay[k] = FALSE;

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Skip "uncreated" artifacts */
		if (!a_ptr->cur_num) continue;

		/* Skip "unknown" artifacts */
		if (!a_ptr->known && !admin) continue;

		/* Skip "hidden" artifacts */
		if (admin_artifact_p(&forge) && !admin) continue;

		/* Assume okay */
		okay[k] = TRUE;
	}

	/* Check the inventories */
	for (i = 1; i <= NumPlayers; i++) {
		p_ptr = Players[i];

		/* Check this guy's */
		for (j = 0; j < INVEN_PACK; j++) {
			o_ptr = &p_ptr->inventory[j];

			/* Ignore non-objects */
			if (!o_ptr->k_idx) continue;

			/* Ignore non-artifacts */
			if (!true_artifact_p(o_ptr)) continue;

			/* Ignore known items */
			if (object_known_p(Ind, o_ptr) || admin) continue;

			/* Skip "hidden" artifacts */
			if (admin_artifact_p(o_ptr) && !admin) continue;

			/* Note the artifact */
			okay[o_ptr->name1] = FALSE;
		}
	}

#ifdef ARTS_PRE_SORT
	/* init */
	memset(radix_buf_cnt, 0, sizeof(int) * 10);
	/* Build radix key, forged from 2 digits of tval and 2 digits of sval */
	z = 0;
	for (i = 0; i < MAX_A_IDX; i++) {
		if (!okay[i]) continue;

		a_ptr = &a_info[i];
		radix_idx[z] = i;
		radix_key[z] = a_ptr->tval * 100 + a_ptr->sval;
		z++;
	}
	/* Sort starting at least significant digit, for all 4 digits */
	for (n = 1; n <= 1000; n *= 10) { /* 10^digit 0..3 */
		for (i = 0; i < z; i++) { /* # of valid arts */
			k = (radix_key[i] / n) % 10;
			j = radix_buf_cnt[k];
			radix_buf[j][k] = radix_key[i];
			radix_buf_idx[j][k] = radix_idx[i];
			radix_buf_cnt[k]++;
		}
		/* re-merge it to prepare sorting the next digit */
		k = 0;
		for (i = 0; i < 10; i++) {
			for (j = 0; j < radix_buf_cnt[i]; j++) {
				radix_key[k] = radix_buf[j][i];
				radix_idx[k] = radix_buf_idx[j][i];
				k++;
			}
			/* empty bucket */
			radix_buf_cnt[i] = 0;
		}
	}
#endif

	/* Scan the artifacts */
#ifndef ARTS_PRE_SORT
	for (i = 0; i < MAX_A_IDX; i++) {
		a_ptr = &a_info[i];

		/* List "dead" ones */
		if (!okay[i]) continue;
#else
	for (i = 0; i < z; i++) {
		a_ptr = &a_info[radix_idx[i]];
#endif

		/* Paranoia */
		strcpy(base_name, "Unknown Artifact");

		/* Obtain the base object type */
		k = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Real object */
		if (k) {
			/* Create the object */
			invcopy(&forge, k);

			/* Create the artifact */
#ifndef ARTS_PRE_SORT
			forge.name1 = i;
#else
			forge.name1 = radix_idx[i];
#endif

			/* Describe the artifact */
			object_desc_store(Ind, base_name, &forge, FALSE, 0);

			/* Hack -- remove {0} */
			j = strlen(base_name);
			base_name[j - 4] = '\0';

			/* Hack -- Build the artifact name */
			if (admin) {
				char timeleft[9], c = 'w';
				if (a_ptr->timeout <= 0) sprintf(timeleft, "\377s  - ");
				else if (a_ptr->timeout < 60 * 2) sprintf(timeleft, "\377r%3dm", a_ptr->timeout);
				else if (a_ptr->timeout < 60 * 24 * 2) sprintf(timeleft, "\377y%3dh", a_ptr->timeout / 60);
				else sprintf(timeleft, "\377s%3dd", a_ptr->timeout / 60 / 24);

				if (a_ptr->cur_num != 1 && !multiple_artifact_p(&forge)) c = 'r';
				else if (admin_artifact_p(&forge)) c = 'y';
				else if (winner_artifact_p(&forge)) c = 'v';
				else if (a_ptr->flags4 & TR4_SPECIAL_GENE) c = 'B';
				else if (a_ptr->cur_num != 1) c = 'o';
				fprintf(fff, "\377%c", c);
#ifndef ARTS_PRE_SORT
				fprintf(fff, "%3d/%d %s\377%c", i, a_ptr->cur_num, timeleft, c);
#else
				fprintf(fff, "%3d/%d %s\377%c", radix_idx[i], a_ptr->cur_num, timeleft, c);
#endif
			}
			fprintf(fff, "%sThe %s", admin ? " " : "     ", base_name);
			if (admin) {
				sprintf(fmt, "%%%ds%%s\n", (int)(45 - strlen(base_name)));
				if (!a_ptr->known) fprintf(fff, fmt, "", "(unknown)");
				else if (multiple_artifact_p(&forge)) fprintf(fff, "\n");
				else if (a_ptr->carrier) fprintf(fff, fmt, "", lookup_player_name(a_ptr->carrier) ? lookup_player_name(a_ptr->carrier) : "(dead player)");
				else fprintf(fff, fmt, "", "???");
			} else fprintf(fff, "\n");
#ifdef ART_DIZ
//	                fprintf(fff, "%s", a_text + a_info[forge.name1].text);
#endif
		}

		shown = TRUE;
	}

	if (!shown) fprintf(fff, "\377sNo artifacts are witnessed so far.\n");

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Artifacts Seen", line, 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Check the status of "uniques"
 *
 * Note that the player ghosts are ignored.  XXX XXX XXX
 *
 * Any unique seen by any player will be shown.  Also, I plan to add the name
 * of the slayer (if any) to the list, so the others will know just how
 * powerful any certain player is.  --KLJ--
 */
/* Pfft, we should rewrite show_file so that we can change
 * the colour for each letter!	- Jir - */
void do_cmd_check_uniques(int Ind, int line)
{
	monster_race *r_ptr;

	int i, j, kk;
	byte ok;
	bool full;

	int k, l, total = 0, own_highest = 0, own_highest_level = 0;
	byte attr;

	FILE *fff;

	char file_name[MAX_PATH_LENGTH];

	player_type *q_ptr = Players[Ind], *p_ptr = q_ptr;
	bool admin = is_admin(q_ptr);
	s16b idx[MAX_R_IDX];

	char buf[17];


	/* Temporary file */
	if (path_temp(file_name, MAX_PATH_LENGTH)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "wb");

	if (!is_newer_than(&q_ptr->version, 4, 4, 7, 0, 0, 0))
		fprintf(fff, "\377U============== Unique Monster List ==============\n");

	/* Scan the monster races */
	for (k = 1; k < MAX_R_IDX-1; k++) {
		r_ptr = &r_info[k];

		/* Only print Uniques */
		if (r_ptr->flags1 & RF1_UNIQUE) {
			/* Only display known uniques */
//			if (r_ptr->r_sights && mon_allowed(r_ptr))
			if (r_ptr->r_sights && mon_allowed_view(r_ptr))
				idx[total++] = k;

			/* remember highest unique the viewing player actually killed */
			if ((q_ptr->r_killed[k] == 1) && (own_highest_level <= r_ptr->level)) {
				own_highest = k;
				own_highest_level = r_ptr->level;
			}
		}
	}

	if (!own_highest)
		if (!(p_ptr->uniques_alive))
			fprintf(fff, "\377U  (you haven't killed any unique monster so far)\n");

	if (total) {
		/* Setup the sorter */
		ang_sort_comp = ang_sort_comp_mon_lev;
		ang_sort_swap = ang_sort_swap_s16b;

		/* Sort the monsters according to value */
		ang_sort(Ind, &idx, NULL, total);

		/* for each unique */
		for (l = total - 1; l >= 0; l--) {
			j = 0;
			ok = FALSE;
			full = FALSE;

			k = idx[l];
			r_ptr = &r_info[k];

			/* Compact list */
			kk = 0;
			if (p_ptr->uniques_alive && p_ptr->r_killed[k] == 1) {
				if (!p_ptr->party) continue; //No party. (._. )
				for (i = 1; i <= NumPlayers; i++) {
					q_ptr = Players[i];
					if (q_ptr->r_killed[k] == 1) continue;
					if (p_ptr->party != q_ptr->party) continue;
					if ((p_ptr->wpos.wx != q_ptr->wpos.wx) || (p_ptr->wpos.wy != q_ptr->wpos.wy) || (p_ptr->wpos.wz != q_ptr->wpos.wz)) continue;
					kk = 1; break;
				}
				if (!kk) continue;
			}
			
			/* Output color byte */
			fprintf(fff, "\377%c", (p_ptr->r_killed[k] == 1 || kk) ? 'w' : 'D');

			/* Hack -- Show the ID for admin -- and also the level */
			if (admin) fprintf(fff, "(%4d, L%d) ", k, r_ptr->level);

			/* Format message */
//			fprintf(fff, "%s has been killed by:\n", r_name + r_ptr->name);
			/* different colour for uniques higher than Morgoth (the 'boss') */
//			if (r_ptr->level > 100) fprintf(fff, "\377s%s was slain by", r_name + r_ptr->name); else
			if (!(p_ptr->uniques_alive)) {
				if (r_ptr->level == 100) fprintf(fff, "\377v%s was slain by", r_name + r_ptr->name); /* only Morgoth is level 100 ! */
				else fprintf(fff, "%s was slain by", r_name + r_ptr->name);
			} else {
				if (r_ptr->level == 100) fprintf(fff, "\377v%s", r_name + r_ptr->name); /* only Morgoth is level 100 ! */
				else fprintf(fff, "%s", r_name + r_ptr->name);
			}
			
			for (i = 1; i <= NumPlayers; i++) {
				q_ptr = Players[i];

				/* don't display dungeon master to players */
				if (q_ptr->admin_dm && !p_ptr->admin_dm) continue;
				
				if (p_ptr->uniques_alive) {
					if (p_ptr->id == q_ptr->id) continue;
					if (!q_ptr->party) continue;
					if (q_ptr->r_killed[k] == 1) continue;
					if (p_ptr->party != q_ptr->party) continue;
					if ((p_ptr->wpos.wx != q_ptr->wpos.wx) || (p_ptr->wpos.wy != q_ptr->wpos.wy) || (p_ptr->wpos.wz != q_ptr->wpos.wz)) continue;
					attr = 'D';
					
					/* first player name entry for this unique? add ':' and go to next line */
					if (!ok) {
						fprintf(fff, ":\n");
						ok = TRUE;
					}

					/* add this player name as entry */
					fprintf(fff, "\377%c", attr);
					sprintf(buf, "(%.14s)", q_ptr->name);
					fprintf(fff, "  %-16.16s", buf);

					/* after 4 entries per line go to next line */
					j++;
					full = FALSE;
					if (j == 4) {
						fprintf(fff, "\n");
						j = 0;
						full = TRUE;
					}
					
					continue;
				}
				else if (q_ptr->r_killed[k] == 1) {
					/* killed it himself */

					attr = 'B';
					/* Print self in green */
					if (Ind == i) attr = 'G';

					/* Print party members in blue */
//					else if (p_ptr->party && p_ptr->party == q_ptr->party) attr = 'B';

					/* Print hostile players in red */
//					else if (check_hostile(Ind, i)) attr = 'r';

					/* first player name entry for this unique? add ':' and go to next line */
					if (!ok) {
						fprintf(fff, ":\n");
						ok = TRUE;
					}

					/* add this player name as entry */
					fprintf(fff, "\377%c", attr);
					fprintf(fff, "  %-16.16s", q_ptr->name);

					/* after 4 entries per line go to next line */
					j++;
					full = FALSE;
					if (j == 4) {
						fprintf(fff, "\n");
						j = 0;
						full = TRUE;
					}
				}
//				else if (Ind == i && q_ptr->r_killed[k] == 2) {
//					/* helped killing it - only shown to the player who helped */
				else if (q_ptr->r_killed[k] == 2) {
					/* helped killing it */

					attr = 'D';
					if (Ind == i) attr = 's';

					/* first player name entry for this unique? add ':' and go to next line */
					if (!ok) {
						fprintf(fff, ":\n");
						ok = TRUE;
					}

					/* add this player name as entry */
					fprintf(fff, "\377%c", attr);
					sprintf(buf, "(%.14s)", q_ptr->name);
					fprintf(fff, "  %-16.16s", buf);

					/* after 4 entries per line go to next line */
					j++;
					full = FALSE;
					if (j == 4) {
						fprintf(fff, "\n");
						j = 0;
						full = TRUE;
					}
				}
			}

			/* not killed by anybody yet? */
			if (!(p_ptr->uniques_alive)) {
				if (!ok) {
					if (r_ptr->r_tkills) fprintf(fff, " somebody.");
					else fprintf(fff, " \377Dnobody.");
				}
			}

			/* Terminate line */
			if (!full) fprintf(fff, "\n");

			/* extra marker line to show where our glory ends for the moment */
			if (!(p_ptr->uniques_alive))
			if (own_highest && own_highest == k) {
				fprintf(fff, "\377U  (strongest unique monster you killed)\n");
				/* only display this marker once */
				own_highest = 0;
			}
		}
	} else {
		if (!(p_ptr->uniques_alive)) {
			fprintf(fff, "\377w");
			fprintf(fff, "No uniques were witnessed so far.\n");
		}
	}

	/* finally.. */
	if (!is_newer_than(&q_ptr->version, 4, 4, 7, 0, 0, 0))
		fprintf(fff, "\377U========== End of Unique Monster List ==========\n");

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Unique Monster List", line, 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}

/* KEEP THESE ANTI_MAXPLV_ DEFINES CONSISTENT WITH party.c! */
#define ANTI_MAXPLV_EXPLOIT
#define ANTI_MAXPLV_EXPLOIT_SOFTLEV
static void do_write_others_attributes(int Ind, FILE *fff, player_type *q_ptr, char attr, bool admin)
{
	player_type *p_ptr = Players[Ind];
	int modify_number = 0, compaction = (p_ptr->player_list ? 2 : 0) + (p_ptr->player_list2 ? 1 : 0);
	cptr p = "";
	char info_chars[4];
	bool text_pk = FALSE, text_silent = FALSE, text_afk = FALSE, text_ignoring_chat = FALSE, text_allow_dm_chat = FALSE;
	bool iddc = in_irondeepdive(&q_ptr->wpos) || (q_ptr->mode & MODE_DED_IDDC);
	bool iddc0 = in_irondeepdive(&p_ptr->wpos) || (p_ptr->mode & MODE_DED_IDDC);
	bool cant_iddc = !iddc && q_ptr->max_exp;
	bool cant_iddc0 = !iddc0 && p_ptr->max_exp;
	char attr_p[3];
	bool wont_get_exp;

	/* NOTE: This won't work well with ANTI_MAXPLV_EXPLOIT code except for
	         ANTI_MAXPLV_EXPLOIT_SOFTLEV, which fortunately is the default.
	   NOTE2: Some of these rules might produce asymmetrical colouring,
	          because they ask 'will _I_ get exp from _his_ kills'. */
	wont_get_exp = 
	    ((p_ptr->total_winner && !(q_ptr->total_winner || q_ptr->once_winner)) ||
	    (q_ptr->total_winner && !(p_ptr->total_winner || p_ptr->once_winner)) ||
#ifdef ANTI_MAXPLV_EXPLOIT
 #ifdef ANTI_MAXPLV_EXPLOIT_SOFTLEV
	    (p_ptr->total_winner && ABS(p_ptr->max_lev - (q_ptr->max_plv - (q_ptr->max_plv - q_ptr->max_lev) / 2)) > MAX_KING_PARTY_LEVEL_DIFF) ||
	    (!p_ptr->total_winner && ABS(p_ptr->max_lev - (q_ptr->max_plv - (q_ptr->max_plv - q_ptr->max_lev) / 2)) > MAX_PARTY_LEVEL_DIFF));
 #else
  #ifdef ANTI_MAXPLV_EXPLOIT_SOFTEXP
	    /* let's just have no actual effect on level here */
	    (p_ptr->total_winner && ABS(p_ptr->max_lev - q_ptr->max_lev) > MAX_KING_PARTY_LEVEL_DIFF) ||
		    (!p_ptr->total_winner && ABS(p_ptr->max_lev - q_ptr->max_lev) > MAX_PARTY_LEVEL_DIFF));
  #else
	    (p_ptr->total_winner && ABS(p_ptr->max_lev - q_ptr->max_plv) > MAX_KING_PARTY_LEVEL_DIFF) ||
	    (!p_ptr->total_winner && ABS(p_ptr->max_lev - q_ptr->max_plv) > MAX_PARTY_LEVEL_DIFF));
  #endif
 #endif
#else
	    (p_ptr->total_winner && ABS(p_ptr->max_lev - q_ptr->max_lev) > MAX_KING_PARTY_LEVEL_DIFF) ||
	    (!p_ptr->total_winner && ABS(p_ptr->max_lev - q_ptr->max_lev) > MAX_PARTY_LEVEL_DIFF));
#endif

	attr_p[0] = 0;
	if (attr == 'w') {
		/* display level in light blue for partyable players */
		if (!wont_get_exp &&
		    !compat_pmode(Ind, q_ptr->Ind, FALSE) &&
		    !((iddc && cant_iddc0) || (iddc0 && cant_iddc))) /* if one of them is in iddc and the other cant go there, we cant party */
			strcpy(attr_p, "\377B");
#ifdef IDDC_CHAR_COLOUR_INDICATOR
		if (iddc) attr = 's';
#endif
	} else if (attr == 'B') {
		/* display level in grey for party members out of our exp-sharing range. */
		if (wont_get_exp) strcpy(attr_p, "\377s");
	}

	/* Prepare title at this point already */
	p = get_ptitle(q_ptr, FALSE);

if (compaction == 1 || compaction == 2) { /* #ifdef COMPACT_PLAYERLIST */
 if (compaction != 2) { /* #ifndef COMPACT_ALT */
	/* Print a message */
	fprintf(fff," ");
	if (q_ptr->admin_dm) {
		if (q_ptr->male) fprintf(fff,"\377bDungeon Master ");
		else fprintf(fff,"\377bDungeon Mistress ");
	} else if (q_ptr->admin_wiz) fprintf(fff,"\377bDungeon Wizard ");
	else if (q_ptr->mode & MODE_PVP) fprintf(fff, "\377%cGladiator ", COLOUR_MODE_PVP);
	else if (q_ptr->ghost) fprintf(fff, "\377rGhost ");
	else if (q_ptr->total_winner) {
		fprintf(fff, "\377v%s ",
		    q_ptr->iron_winner ? (q_ptr->male ? "Iron Emperor" : "Iron Empress") :
		    ((q_ptr->mode & (MODE_HARD | MODE_NO_GHOST)) ?
			(q_ptr->male ? "Emperor" : "Empress") :
			(q_ptr->male ? "King" : "Queen")));
	}
	else if (q_ptr->iron_winner) fprintf(fff, "\377%cIron Champion ", attr);
	else fprintf(fff, "\377%c", attr);

	fprintf(fff, "%s, %sL%d \377%c", q_ptr->name, attr_p, q_ptr->lev, attr);

	fprintf(fff, "%s %s", get_prace(q_ptr),  p);

	/* PK */
	if (cfg.use_pk_rules == PK_RULES_DECLARE) {
		text_pk = TRUE;
		if(q_ptr->pkill & (PKILL_SET | PKILL_KILLER))
			fprintf(fff, "  (PK");
		else if(!(q_ptr->pkill & PKILL_KILLABLE))
			fprintf(fff, "  (SAFE");
		else if(!(q_ptr->tim_pkill))
			fprintf(fff, q_ptr->lev < 5 ? "  (Newbie" : "  (Killable");
		else
			text_pk = FALSE;
	}
	if (q_ptr->limit_chat) {
		text_silent = TRUE;
		if (text_pk)
			fprintf(fff, ", Silent");
		else
			fprintf(fff, "  (Silent");
	}
	/* AFK */
	if (q_ptr->afk) {
		text_afk = TRUE;
		if (text_pk || text_silent) {
//			if (strlen(q_ptr->afk_msg) == 0)
				fprintf(fff, ", AFK");
//			else
//				fprintf(fff, ", AFK: %s", q_ptr->afk_msg);
		} else {
//			if (strlen(q_ptr->afk_msg) == 0)
				fprintf(fff, "  (AFK");
//			else
//				fprintf(fff, "   (AFK: %s", q_ptr->afk_msg);
		}
	}
	/* Ignoring normal chat (sees only private & party messages) */
	if (q_ptr->ignoring_chat) {
		text_ignoring_chat = TRUE;
		if (text_pk || text_silent || text_afk) {
			fprintf(fff, ", Private mode");
		} else {
			fprintf(fff, "  (Private mode");
		}
	}
	if (q_ptr->admin_dm_chat) {
		text_allow_dm_chat = TRUE;
		if (text_pk || text_silent || text_afk || text_ignoring_chat) {
			fprintf(fff, ", Allow chat");
		} else {
			fprintf(fff, "  (Allow chat");
		}
	}
	if (text_pk || text_silent || text_afk || text_ignoring_chat || text_allow_dm_chat) fprintf(fff, ")");

	/* Line break here, it's getting too long with all those mods -C. Blue */
	fprintf(fff, "\n   \377");

	if (q_ptr->fruit_bat == 1)
		strcpy(info_chars, format("\377%cb", color_attr_to_char(q_ptr->cp_ptr->color)));
	else
		strcpy(info_chars, format("\377%c@", color_attr_to_char(q_ptr->cp_ptr->color)));

	switch (q_ptr->mode & MODE_MASK) { // TODO: give better modifiers
		default:
		case MODE_NORMAL:
			fprintf(fff, "W");
			break;
		case MODE_EVERLASTING:
			fprintf(fff, "B");
			break;
		case MODE_PVP:
			fprintf(fff, "%c", COLOUR_MODE_PVP);
			break;
		case (MODE_HARD | MODE_NO_GHOST):
			fprintf(fff, "r");
			break;
		case MODE_HARD:
			fprintf(fff, "s");
			break;
		case MODE_NO_GHOST:
			fprintf(fff, "D");
			break;
	}

	fprintf(fff, "*%s\377U ", info_chars);
	fprintf(fff, "%s", q_ptr->male ? "male" : "female");
	if (q_ptr->fruit_bat == 1) fprintf(fff, " bat"); /* only for true battys, not polymorphed ones */
//	if (admin) fprintf(fff, " (%s@%s)", q_ptr->accountname, q_ptr->hostname); else
	fprintf(fff, " (%s)", q_ptr->accountname);
	if (q_ptr->guild || q_ptr->party) fprintf(fff, ",");

	if (q_ptr->guild) {
		fprintf(fff, " \377y[\377%c%s\377y]\377U", COLOUR_CHAT_GUILD, guilds[q_ptr->guild].name);
	}
	if (q_ptr->party) {
		fprintf(fff, " '%s%s\377U'",
		(parties[q_ptr->party].mode == PA_IRONTEAM) ? "\377s" : "",
		parties[q_ptr->party].name);
	}
 } else { /* COMPACT_ALT */
	/* Print a message */
	fprintf(fff," ");
	if (q_ptr->admin_dm) {
		if (q_ptr->male) fprintf(fff,"\377bDungeon Master ");
		else fprintf(fff,"\377bDungeon Mistress ");
	} else if (q_ptr->admin_wiz) fprintf(fff,"\377bDungeon Wizard ");
	else if (q_ptr->mode & MODE_PVP) fprintf(fff, "\377%cGladiator ", COLOUR_MODE_PVP);
	else if (q_ptr->ghost) fprintf(fff, "\377rGhost ");
	else if (q_ptr->total_winner) {
		fprintf(fff, "\377v%s ",
		    q_ptr->iron_winner ? (q_ptr->male ? "Iron Emperor" : "Iron Empress") :
		    ((q_ptr->mode & (MODE_HARD | MODE_NO_GHOST)) ?
			(q_ptr->male ? "Emperor" : "Empress") :
			(q_ptr->male ? "King" : "Queen")));
	}
	else if (q_ptr->iron_winner) fprintf(fff, "\377%cIron Champion ", attr);
	else fprintf(fff, "\377%c", attr);

  #ifdef COMPACT_GENDER
	fprintf(fff, "%s,\377%c %c.%sL%d\377%c ", q_ptr->name, attr, q_ptr->male ? 'm' : 'f', attr_p, q_ptr->lev, attr);
  #else
	fprintf(fff, "%s, %sL%d\377%c %s ", q_ptr->name, attr_p, q_ptr->lev, attr, q_ptr->male ? "Male" : "Female");
  #endif

	fprintf(fff, "%s %s", get_prace(q_ptr),  p);

	/* PK */
	if (cfg.use_pk_rules == PK_RULES_DECLARE) {
		text_pk = TRUE;
		if(q_ptr->pkill & (PKILL_SET | PKILL_KILLER))
			fprintf(fff, "  (PK");
		else if(!(q_ptr->pkill & PKILL_KILLABLE))
			fprintf(fff, "  (SAFE");
		else if(!(q_ptr->tim_pkill))
			fprintf(fff, q_ptr->lev < 5 ? "  (Newbie" : "  (Killable");
		else
			text_pk = FALSE;
	}
	if (q_ptr->limit_chat) {
		text_silent = TRUE;
		if (text_pk)
			fprintf(fff, ", Silent");
		else
			fprintf(fff, "  (Silent");
	}
	/* AFK */
	if (q_ptr->afk) {
		text_afk = TRUE;
		if (text_pk || text_silent) {
//			if (strlen(q_ptr->afk_msg) == 0)
				fprintf(fff, ", AFK");
//			else
//				fprintf(fff, ", AFK: %s", q_ptr->afk_msg);
		} else {
//			if (strlen(q_ptr->afk_msg) == 0)
				fprintf(fff, "  (AFK");
//			else
//				fprintf(fff, "   (AFK: %s", q_ptr->afk_msg);
		}
	}
	/* Ignoring normal chat (sees only private & party messages) */
	if (q_ptr->ignoring_chat) {
		text_ignoring_chat = TRUE;
		if (text_pk || text_silent || text_afk) {
			fprintf(fff, ", Private mode");
		} else {
			fprintf(fff, "  (Private mode");
		}
	}
	if (q_ptr->admin_dm_chat) {
		text_allow_dm_chat = TRUE;
		if (text_pk || text_silent || text_afk || text_ignoring_chat) {
			fprintf(fff, ", Allow chat");
		} else {
			fprintf(fff, "  (Allow chat");
		}
	}
	if (text_pk || text_silent || text_afk || text_ignoring_chat || text_allow_dm_chat) fprintf(fff, ")");

	/* Line break here, it's getting too long with all those mods -C. Blue */
	fprintf(fff, "\n   \377");

	if (q_ptr->fruit_bat == 1)
		strcpy(info_chars, format("\377%cb", color_attr_to_char(q_ptr->cp_ptr->color)));
	else
		strcpy(info_chars, format("\377%c@", color_attr_to_char(q_ptr->cp_ptr->color)));

	switch (q_ptr->mode & MODE_MASK) { // TODO: give better modifiers
		default:
		case MODE_NORMAL:
			fprintf(fff, "W");
			break;
		case MODE_EVERLASTING:
			fprintf(fff, "B");
			break;
		case MODE_PVP:
			fprintf(fff, "%c", COLOUR_MODE_PVP);
			break;
		case (MODE_HARD | MODE_NO_GHOST):
			fprintf(fff, "r");
			break;
		case MODE_HARD:
			fprintf(fff, "s");
			break;
		case MODE_NO_GHOST:
			fprintf(fff, "D");
			break;
	}

	fprintf(fff, "*%s\377U", info_chars);
	fprintf(fff, " (%s@%s)", q_ptr->accountname, q_ptr->hostname);

	if (q_ptr->guild) {
		fprintf(fff, ", \377y[\377%c%s\377y]\377U", COLOUR_CHAT_GUILD, guilds[q_ptr->guild].name);
	}
	if (q_ptr->party) {
		if (!q_ptr->guild) fprintf(fff, ", Party:");
		fprintf(fff, " '%s%s\377U'",
		(parties[q_ptr->party].mode == PA_IRONTEAM) ? "\377s" : "",
		parties[q_ptr->party].name);
	}
 }

} else { /* COMPACT_PLAYERLIST */
 if (compaction == 3) { /* #ifdef ULTRA_COMPACT_PLAYERLIST */
	char flag_str[10];

	/* Print a message */
	fprintf(fff," ");
	if (is_admin(q_ptr)) fprintf(fff,"\377b");
	else if (q_ptr->mode & MODE_PVP) fprintf(fff, "\377%c", COLOUR_MODE_PVP);
	else if (q_ptr->ghost) fprintf(fff, "\377r");
	else if (q_ptr->total_winner) fprintf(fff, "\377v");
	else fprintf(fff, "\377%c", attr);

  #ifdef COMPACT_GENDER
	fprintf(fff, "%s,\377%c %c.%sL%d\377%c ", q_ptr->name, attr, q_ptr->male ? 'm' : 'f', attr_p, q_ptr->lev, attr);
  #else
	fprintf(fff, "%s, %sL%d\377%c ", q_ptr->name, attr_p, q_ptr->lev, attr);
  #endif

	fprintf(fff, "%s", get_prace(q_ptr));
	fprintf(fff, " %s", class_info[q_ptr->pclass].title);

	/* location */
	if (attr == 'G' || attr == 'B' || admin
#ifdef IDDC_CHAR_POSITION_INDICATOR
	    || iddc
#endif
	    ) {
		// BAD HACK: just replacing 'Ind' by number constants..
  #if 0 /* 'The Sacred Land of Mountains' <- too long for this ultra compact scheme! */
		if (admin) fprintf(fff, ", %s", wpos_format(1, &q_ptr->wpos));
		else fprintf(fff, ", %s", wpos_format(-1, &q_ptr->wpos));
  #else /* ..so give everyone exact wpos, like otherwise only admins get */
		fprintf(fff, ", %s", wpos_format_compact(Ind, &q_ptr->wpos));
  #endif

		fprintf(fff, " [%d,%d]", q_ptr->panel_row, q_ptr->panel_col);

		/* Quest flag */
//		fprintf(fff, " %c", (q_ptr->quest_id ? 'Q' : ' '));
	}

	/* PK */
	if (cfg.use_pk_rules == PK_RULES_DECLARE) {
		text_pk = TRUE;
		if(q_ptr->pkill & (PKILL_SET | PKILL_KILLER))
			fprintf(fff, " (PK");
		else if(!(q_ptr->pkill & PKILL_KILLABLE))
			fprintf(fff, " (SAFE");
		else if(!(q_ptr->tim_pkill))
			fprintf(fff, q_ptr->lev < 5 ? " (New" : " (Kill");
		else
			text_pk = FALSE;
	}
	if (q_ptr->limit_chat) {
		text_silent = TRUE;
		if (text_pk)
			fprintf(fff, ", Silent");
		else
			fprintf(fff, " (Silent");
	}
	/* AFK */
	if (q_ptr->afk) {
		text_afk = TRUE;
		if (text_pk || text_silent) {
				fprintf(fff, ", AFK");
		} else {
				fprintf(fff, " (AFK");
		}
	}
	/* Ignoring normal chat (sees only private & party messages) */
	if (q_ptr->ignoring_chat) {
		text_ignoring_chat = TRUE;
		if (text_pk || text_silent || text_afk) {
			fprintf(fff, ", Private");
		} else {
			fprintf(fff, " (Private");
		}
	}
	if (q_ptr->admin_dm_chat) {
		text_allow_dm_chat = TRUE;
		if (text_pk || text_silent || text_afk || text_ignoring_chat) {
			fprintf(fff, ", Chat");
		} else {
			fprintf(fff, " (Chat");
		}
	}
	if (text_pk || text_silent || text_afk || text_ignoring_chat || text_allow_dm_chat) fprintf(fff, ")");


	/* 2nd line */
	if (q_ptr->inval) {
		if (q_ptr->v_unknown && admin) strcpy(flag_str, "\377yI\377rU");
		else if (q_ptr->v_test && admin) strcpy(flag_str, "\377yI\377oT");
		else if (q_ptr->v_outdated) strcpy(flag_str, "\377yI\377DO");
		else if (!q_ptr->v_latest && admin) strcpy(flag_str, "\377yI\377sL");
		else strcpy(flag_str, "\377yI ");
	} else {
		if (q_ptr->v_unknown && admin) strcpy(flag_str, "\377rU ");
		else if (q_ptr->v_test && admin) strcpy(flag_str, "\377oT ");
		else if (q_ptr->v_outdated) strcpy(flag_str, "\377DO ");
		else if (!q_ptr->v_latest && admin) strcpy(flag_str, "\377sL ");
		else strcpy(flag_str, "  ");
	}
	fprintf(fff, "\n %s\377", flag_str);


	if (q_ptr->fruit_bat == 1)
		strcpy(info_chars, format("\377%cb", color_attr_to_char(q_ptr->cp_ptr->color)));
	else
		strcpy(info_chars, format("\377%c@", color_attr_to_char(q_ptr->cp_ptr->color)));

	switch (q_ptr->mode & MODE_MASK) { // TODO: give better modifiers
		default:
		case MODE_NORMAL:
			fprintf(fff, "W");
			break;
		case MODE_EVERLASTING:
			fprintf(fff, "B");
			break;
		case MODE_PVP:
			fprintf(fff, "%c", COLOUR_MODE_PVP);
			break;
		case (MODE_HARD | MODE_NO_GHOST):
			fprintf(fff, "r");
			break;
		case MODE_HARD:
			fprintf(fff, "s");
			break;
		case MODE_NO_GHOST:
			fprintf(fff, "D");
			break;
	}

	fprintf(fff, "*%s\377U", info_chars);
//	fprintf(fff, " (%s@%s)", q_ptr->accountname, q_ptr->hostname);
	fprintf(fff, " (%s@%s)", q_ptr->accountname, q_ptr->hostname);

  #ifndef COMPACT_GENDER
	fprintf(fff, ", %s", q_ptr->male ? "Male" : "Female");
  #endif

	/* overlapping AFK msg with guild/party names */
	if ((!q_ptr->afk) || !strlen(q_ptr->afk_msg)) {
		if (!q_ptr->info_msg[0]) {
			if (q_ptr->guild)
				fprintf(fff, ", \377y[\377%c%s\377y]\377U", COLOUR_CHAT_GUILD, guilds[q_ptr->guild].name);
			if (q_ptr->party) {
				if (!q_ptr->guild) fprintf(fff, ", Party:");
				fprintf(fff, " '%s%s\377U'",
				(parties[q_ptr->party].mode == PA_IRONTEAM) ? "\377s" : "",
				parties[q_ptr->party].name);
			}
		} else fprintf(fff, "  \377U(%s\377U)", q_ptr->info_msg);
	} else fprintf(fff, "  \377u(%s\377u)", q_ptr->afk_msg);

 } else { //#else
	/* Check for special character */
	/* Uncomment these as you feel it's needed ;) */
	//if(!strcmp(q_ptr->name,"")) modify_number=1; //wussy Cheezer
	//if(!strcmp(q_ptr->name,"")) modify_number=2; //silyl Slacker
	//if(!strcmp(q_ptr->name,"Duncan McLeod")) modify_number=3; //Highlander games Judge ;) Bread and games to them!!
	//if(!strcmp(q_ptr->name,"Tomenet")) modify_number=4;//Server-specific Dungeon Masters
	//if(!strcmp(q_ptr->name,"C. Blue")) modify_number=4;//Server-specific Dungeon Masters
	//if(!strcmp(q_ptr->name,"C.Blue")) modify_number=4;//Server-specific Dungeon Masters
	if(q_ptr->admin_dm) modify_number=4;
	if(q_ptr->admin_wiz) modify_number=5;

	/* Print a message */
  #if 0
	fprintf(fff, "  %s the %s%s %s (%s%sLv %d, %s)",
			q_ptr->name, (q_ptr->mode == MODE_HARD)?"hellish ":"",
			race_info[q_ptr->prace].title, class_info[q_ptr->pclass].title,
			(q_ptr->total_winner)?
			    ((p_ptr->mode & (MODE_HARD | MODE_NO_GHOST))?
				((q_ptr->male)?"Emperor":"Empress"):
				((q_ptr->male)?"King, ":"Queen, ")):
			    ((q_ptr->male)?"Male, ":"Female, "),
			q_ptr->fruit_bat ? "Fruit bat, " : "",
			q_ptr->lev, parties[q_ptr->party].name);
  #else	// 0
   #ifndef ABUNDANT_TITLES
	fprintf(fff, "  %s the ", q_ptr->name);
	switch(modify_number){
	case 1:	fprintf(fff, "wussy "); break;
	case 2: fprintf(fff, "silyl "); break;
	default:
		switch (q_ptr->mode & MODE_MASK)	// TODO: give better modifiers
		{
			case MODE_NORMAL:
				break;
			case MODE_HARD:
				fprintf(fff, "purgatorial ");
				break;
			case MODE_NO_GHOST:
				fprintf(fff, "unworldly ");
				break;
			case MODE_EVERLASTING:
				fprintf(fff, "everlasting ");
				break;
	    		case (MODE_HARD + MODE_NO_GHOST):
				fprintf(fff, "hellish ");
				break;
		}
		break;
	}

	switch (modify_number) {
	case 3: fprintf(fff, "Highlander "); break; //Judge for Highlander games
	default: fprintf(fff, "%s ", race_info[q_ptr->prace].title); break;
	}

	switch (modify_number) {
	case 1: fprintf(fff, "Cheezer "); break;
	case 2: fprintf(fff, "Slacker "); break;
	case 3: if (q_ptr->male) fprintf(fff, "Swordsman ");
		else fprintf(fff, "Swordswoman ");
		break; //Judge for Highlander games
	default:
		fprintf(fff, "%s", class_info[q_ptr->pclass].title); break;
	}

	if (q_ptr->mode & MODE_PVP) fprintf(fff, " Gladiator");
   #else
    #if 0
	switch (q_ptr->mode & MODE_MASK)	// TODO: give better modifiers
	{
		case MODE_NORMAL:
			break;
		case MODE_HARD:
			fprintf(fff, "\377s");
			break;
		case MODE_NO_GHOST:
			fprintf(fff, "\377D");
			break;
		case MODE_EVERLASTING:
			fprintf(fff, "\377B");
			break;
		case MODE_PVP:
			fprintf(fff, "\377%c", COLOUR_MODE_PVP);
			break;
		case (MODE_HARD | MODE_NO_GHOST):
			fprintf(fff, "\377s");
			break;
	}
	fprintf(fff, "  %s\377%c the ", q_ptr->name, attr);
    #else
	fprintf(fff, "  %s the ", q_ptr->name);
    #endif
	fprintf(fff, "%s %s", get_prace(q_ptr),  p); 
   #endif
	if (q_ptr->mode & MODE_PVP) fprintf(fff, " Gladiator");

	/* PK */
	if (cfg.use_pk_rules == PK_RULES_DECLARE) {
		text_pk = TRUE;
		if(q_ptr->pkill & (PKILL_SET | PKILL_KILLER))
			fprintf(fff, "   (PK");
		else if(!(q_ptr->pkill & PKILL_KILLABLE))
			fprintf(fff, "   (SAFE");
		else if(!(q_ptr->tim_pkill))
			fprintf(fff, q_ptr->lev < 5 ? "   (Newbie" : "   (Killable");
		else
			text_pk = FALSE;
	}
	if (q_ptr->limit_chat) {
		text_silent = TRUE;
		if (text_pk)
			fprintf(fff, ", Silent");
		else
			fprintf(fff, "   (Silent");
	}
	/* AFK */
	if (q_ptr->afk) {
		text_afk = TRUE;
		if (text_pk || text_silent) {
//			if (strlen(q_ptr->afk_msg) == 0)
				fprintf(fff, ", AFK");
//			else
//				fprintf(fff, ", AFK: %s", q_ptr->afk_msg);
		} else {
//			if (strlen(q_ptr->afk_msg) == 0)
				fprintf(fff, "   (AFK");
//			else
//				fprintf(fff, "   (AFK: %s", q_ptr->afk_msg);
		}
	}
	/* Ignoring normal chat (sees only private & party messages) */
	if (q_ptr->ignoring_chat) {
		text_ignoring_chat = TRUE;
		if (text_pk || text_silent || text_afk) {
			fprintf(fff, ", Private mode");
		} else {
			fprintf(fff, "   (Private mode");
		}
	}
	if (q_ptr->admin_dm_chat) {
		text_allow_dm_chat = TRUE;
		if (text_pk || text_silent || text_afk || text_ignoring_chat) {
			fprintf(fff, ", Allow chat");
		} else {
			fprintf(fff, "   (Allow chat");
		}
	}
	if (text_pk || text_silent || text_afk || text_ignoring_chat || text_allow_dm_chat) fprintf(fff, ")");

	/* Line break here, it's getting too long with all that mods -C. Blue */
   #ifdef ABUNDANT_TITLES
    #if 0
	strcpy(info_chars, " "));
    #else
	if (q_ptr->fruit_bat == 1)
		strcpy(info_chars, format("\377%cb", color_attr_to_char(q_ptr->cp_ptr->color)));
	else
		strcpy(info_chars, format("\377%c@", color_attr_to_char(q_ptr->cp_ptr->color)));
    #endif
	switch (q_ptr->mode & MODE_MASK)	// TODO: give better modifiers
	{
		case MODE_NORMAL:
			fprintf(fff, "\n\377W  *%s\377U ", info_chars);
			break;
		case MODE_EVERLASTING:
			fprintf(fff, "\n\377B  *%s\377U ", info_chars);
			break;
		case MODE_PVP:
			fprintf(fff, "\n\377%c  *%s\377U ", COLOUR_MODE_PVP, info_chars);
			break;
		case (MODE_HARD | MODE_NO_GHOST):
			fprintf(fff, "\n\377r  *%s\377U ", info_chars);
			break;
		case MODE_HARD:
			fprintf(fff, "\n\377s  *%s\377U ", info_chars);
			break;
		case MODE_NO_GHOST:
			fprintf(fff, "\n\377D  *%s\377U ", info_chars);
			break;
	}
   #else
	fprintf(fff, "\n\377U     ");
   #endif

	switch (modify_number) {
	case 3: fprintf(fff, "\377rJudge\377U "); break; //Judge for Highlander games
	case 4: if (q_ptr->male) fprintf(fff,"\377bDungeon Master\377U ");
		else fprintf(fff,"\377bDungeon Mistress\377U ");
		break; //Server Admin
	case 5: if (q_ptr->male) fprintf(fff,"\377bDungeon Wizard\377U ");
		else fprintf(fff,"\377bDungeon Wizard\377U ");
		break; //Server Admin
	default: fprintf(fff, "%s",
		q_ptr->ghost ? "\377rGhost\377U " :
		(q_ptr->total_winner ?
		    ((q_ptr->mode & (MODE_HARD | MODE_NO_GHOST)) ?
			(q_ptr->male ? "\377vEmperor\377U " : "\377vEmpress\377U ") :
		        (q_ptr->male ? "\377vKing\377U " : "\377vQueen\377U "))
		: (q_ptr->male ? "Male " : "Female ")));
		break;
	}

	fprintf(fff, "%sLv %d\377U", attr_p, q_ptr->lev);
//		q_ptr->fruit_bat == 1 ? "Batty " : "", /* only for true battys, not polymorphed ones */

	if (q_ptr->guild)
		fprintf(fff, ", \377y[\377%c%s\377y]\377U",
		    COLOUR_CHAT_GUILD, guilds[q_ptr->guild].name);
	if (q_ptr->party)
		fprintf(fff, "%s '%s%s\377U'",
		    q_ptr->guild ? "" : ", Party:",
		    (parties[q_ptr->party].mode == PA_IRONTEAM) ? "\377s" : "",
		    parties[q_ptr->party].name);
  #endif // 0
 } //#endif	/* ULTRA_COMPACT_PLAYERLIST */
}//#endif /* COMPACT_PLAYERLIST */
}

/*
 * Check the status of "players"
 *
 * The player's name, race, class, and experience level are shown.
 */
void do_cmd_check_players(int Ind, int line)
{
	player_type *p_ptr = Players[Ind], *q_ptr;
	int k, lines = 0, compaction = (p_ptr->player_list ? 2 : 0) + (p_ptr->player_list2 ? 1 : 0) ;
	FILE *fff;
	char file_name[MAX_PATH_LENGTH];

	bool admin = is_admin(p_ptr);
	char flag_str[10];
	bool iddc;
	bool big_map = (p_ptr->screen_hgt != SCREEN_HGT); //BIG_MAP is currently turned on for this player?

	/* Temporary file */
	if (path_temp(file_name, MAX_PATH_LENGTH)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "wb");
	if(fff == (FILE*)NULL) return;

	/* Scan the player races */
	for (k = 1; k < NumPlayers + 1; k++) {
		q_ptr = Players[k];
		flag_str[0] = '\0';
		byte attr = 'w';

		/* Only print connected players */
		if (q_ptr->conn == NOT_CONNECTED)
			continue;

		/* don't display the dungeon master if the secret_dungeon_master
		 * option is set 
		 */
		if (q_ptr->admin_dm &&
		   (cfg.secret_dungeon_master) && !admin) continue;

		iddc = in_irondeepdive(&q_ptr->wpos) || (q_ptr->mode & MODE_DED_IDDC);

if (compaction == 1 || compaction == 2) { //#ifdef COMPACT_PLAYERLIST
 if (compaction != 2) { //#ifndef COMPACT_ALT
		/*** Determine color ***/
		/* Print self in green */
		if (Ind == k) attr = 'G';
		/* Print other PvP-mode chars in orange */
		else if ((p_ptr->mode & MODE_PVP) && (q_ptr->mode & MODE_PVP)) attr = COLOUR_MODE_PVP;
		/* Print party members in blue */
		else if (p_ptr->party && p_ptr->party == q_ptr->party) attr = 'B';
		/* Print hostile players in red */
		else if (check_hostile(Ind, k)) attr = 'r';
#ifdef IDDC_CHAR_COLOUR_INDICATOR
		if (attr == 'w' && iddc) attr = 's';
#endif

		/* Print a message */
		do_write_others_attributes(Ind, fff, q_ptr, attr, is_admin(p_ptr));

#if 0
		fprintf(fff, "\n   %s", q_ptr->inval ? (!outdated ? (!latest && is_admin(p_ptr) ? "\377yI\377sL \377U" : "\377yI \377U") : "\377yI\377DO \377U") :
		    (outdated ? "\377DO \377U" : (!latest && is_admin(p_ptr) ? "\377sL \377U" :  "\377U")));
#else
		if (q_ptr->inval) strcpy(flag_str, "\377yI");
		if (q_ptr->v_unknown && is_admin(p_ptr)) strcat(flag_str, "\377rU");
		else if (q_ptr->v_test && is_admin(p_ptr)) strcat(flag_str, "\377oT");
		else if (q_ptr->v_outdated) strcat(flag_str, "\377DO");
		else if (!q_ptr->v_latest && is_admin(p_ptr)) strcat(flag_str, "\377sL");
		if (flag_str[0]) strcat(flag_str, " ");
		fprintf(fff, "\n   %s\377U", flag_str);
#endif

		/* Print location if both players are PvP-Mode */
		if (((p_ptr->mode & MODE_PVP) && (q_ptr->mode & MODE_PVP)) && !admin) {
			fprintf(fff, "%s", wpos_format(-Ind, &q_ptr->wpos));
		}
		/* Print extra info if these people are in the same party or if viewer is DM */
		else if ((p_ptr->party == q_ptr->party && p_ptr->party) || Ind == k || admin
#ifdef IDDC_CHAR_POSITION_INDICATOR
		    || iddc
#endif
		    ) {
  #if 1
			if (admin) fprintf(fff, "%s [%d,%d] (%s)", wpos_format(Ind, &q_ptr->wpos), q_ptr->panel_row, q_ptr->panel_col, q_ptr->hostname); else
  #endif
			fprintf(fff, "%s [%d,%d]", wpos_format(-Ind, &q_ptr->wpos), q_ptr->panel_row, q_ptr->panel_col);

			/* Print questing flag */
			if (q_ptr->quest_id) fprintf(fff, " Q");
		}

//		fprintf(fff, ", %s@%s", q_ptr->accountname, q_ptr->hostname);

		/* Print afk/info message */
		if ((!q_ptr->afk) || !strlen(q_ptr->afk_msg)) {
			if (!q_ptr->info_msg[0])
				fprintf(fff, "\n");
			else
				fprintf(fff, "  \377U(%s\377U)\n", q_ptr->info_msg);
		} else
			fprintf(fff, "  \377u(%s\377u)\n", q_ptr->afk_msg);

		lines += 3;
 } else { //#else /* COMPACT_ALT */
		/*** Determine color ***/
		/* Print self in green */
		if (Ind == k) attr = 'G';
		/* Print other PvP-mode chars in orange */
		else if ((p_ptr->mode & MODE_PVP) && (q_ptr->mode & MODE_PVP)) attr = COLOUR_MODE_PVP;
		/* Print party members in blue */
		else if (p_ptr->party && p_ptr->party == q_ptr->party) attr = 'B';
		/* Print hostile players in red */
		else if (check_hostile(Ind, k)) attr = 'r';
#ifdef IDDC_CHAR_COLOUR_INDICATOR
		if (attr == 'w' && iddc) attr = 's';
#endif

		/* Print a message */
		do_write_others_attributes(Ind, fff, q_ptr, attr, is_admin(p_ptr));

#if 0
		fprintf(fff, "\n   %s", q_ptr->inval ? (!outdated ? (!latest && is_admin(p_ptr) ? "\377yI\377U+\377sL \377U" : "\377y(I) \377U") : "\377yI\377U+\377DO \377U") :
		    (outdated ? "\377D(O) \377U" : (!latest && is_admin(p_ptr) ? "\377s(L) \377U" :  "\377U")));
#else
		if (q_ptr->inval) {
			if (q_ptr->v_unknown && is_admin(p_ptr)) strcpy(flag_str, "\377yI\377U+\377rU");
			else if (q_ptr->v_test && is_admin(p_ptr)) strcpy(flag_str, "\377yI\377U+\377oT");
			else if (q_ptr->v_outdated) strcpy(flag_str, "\377yI\377U+\377DO");
			else if (!q_ptr->v_latest && is_admin(p_ptr)) strcpy(flag_str, "\377yI\377U+\377sL");
			else strcpy(flag_str, "\377y(I)");
		} else {
			if (q_ptr->v_unknown && is_admin(p_ptr)) strcpy(flag_str, "\377r(U)");
			else if (q_ptr->v_test && is_admin(p_ptr)) strcpy(flag_str, "\377o(T)");
			else if (q_ptr->v_outdated) strcpy(flag_str, "\377D(O)");
			else if (!q_ptr->v_latest && is_admin(p_ptr)) strcpy(flag_str, "\377s(L)");
		}
		if (flag_str[0]) strcat(flag_str, " ");
		fprintf(fff, "\n   %s\377U", flag_str);
#endif

		/* Print location if both players are PvP-Mode */
		if (((p_ptr->mode & MODE_PVP) && (q_ptr->mode & MODE_PVP)) && !admin) {
			fprintf(fff, "%s", wpos_format(-Ind, &q_ptr->wpos));
		}
		/* Print extra info if these people are in the same party or if viewer is DM */
		else if ((p_ptr->party == q_ptr->party && p_ptr->party) || Ind == k || admin
#ifdef IDDC_CHAR_POSITION_INDICATOR
		    || iddc
#endif
		    ) {
			if (admin) fprintf(fff, "%s [%d,%d]", wpos_format(Ind, &q_ptr->wpos), q_ptr->panel_row, q_ptr->panel_col); else
			fprintf(fff, "%s [%d,%d]", wpos_format(-Ind, &q_ptr->wpos), q_ptr->panel_row, q_ptr->panel_col);

			/* Print questing flag */
			if (q_ptr->quest_id) fprintf(fff, " Q");
		}

		/* Print afk/info message */
		if ((!q_ptr->afk) || !strlen(q_ptr->afk_msg)) {
			if (!q_ptr->info_msg[0])
				fprintf(fff, "\n");
			else
				fprintf(fff, "  \377U(%s\377U)\n", q_ptr->info_msg);
		} else
			fprintf(fff, "  \377u(%s\377u)\n", q_ptr->afk_msg);

		lines += 3;
 } //#endif
} else { //#else /* COMPACT_PLAYERLIST - new way to fit in more info */
 if (compaction == 3) { // #ifdef ULTRA_COMPACT_PLAYERLIST
		/* nothing really! only has 2 lines per entry. */

		/*** Determine color ***/
		/* Print self in green */
		if (Ind == k) attr = 'G';
		/* Print other PvP-mode chars in orange */
		else if ((p_ptr->mode & MODE_PVP) && (q_ptr->mode & MODE_PVP)) attr = COLOUR_MODE_PVP;
		/* Print party members in blue */
		else if (p_ptr->party && p_ptr->party == q_ptr->party) attr = 'B';
		/* Print hostile players in red */
		else if (check_hostile(Ind, k)) attr = 'r';
#ifdef IDDC_CHAR_COLOUR_INDICATOR
		if (attr == 'w' && iddc) attr = 's';
#endif

		do_write_others_attributes(Ind, fff, q_ptr, attr, is_admin(p_ptr));
		fprintf(fff, "\n");

		lines += 2;
 } else { //#else (compaction == 0, ie 4 lines per entry)
		/*** Determine color ***/
		/* Print self in green */
		if (Ind == k) attr = 'G';
		/* Print other PvP-mode chars in orange */
		else if ((p_ptr->mode & MODE_PVP) && (q_ptr->mode & MODE_PVP)) attr = COLOUR_MODE_PVP;
		/* Print party members in blue */
		else if (p_ptr->party && p_ptr->party == q_ptr->party) attr = 'B';
		/* Print hostile players in red */
		else if (check_hostile(Ind, k)) attr = 'r';
#ifdef IDDC_CHAR_COLOUR_INDICATOR
		if (attr == 'w' && iddc) attr = 's';
#endif

		/* Output color byte */
		fprintf(fff, "\377%c", attr);
		/* Print a message */
		do_write_others_attributes(Ind, fff, q_ptr, attr, is_admin(p_ptr));
		/* Colour might have changed due to Iron Team party name,
		   so print the closing ')' in the original colour again: */
		/* not needed anymore since we have linebreak now
		fprintf(fff, "\377%c)", attr);*/

		/* Newline */
		/* -AD- will this work? - Sure -C. Blue- */
		fprintf(fff, "\n\377U");

//		if (is_admin(p_ptr)) fprintf(fff, "  (%d)", k);

		//show local system username? q_ptr->realname
#if 0
		fprintf(fff, " %s (%s@%s) ", q_ptr->inval ? (!outdated ? (!latest && is_admin(p_ptr) ? "\377yI\377U+\377sL\377U" : "\377y(I)\377U") : "\377yI\377U+\377DO\377U") :
		    (outdated ? "\377D(O)\377U" : (!latest && is_admin(p_ptr) ? "\377s(L)\377U" :  "   ")), q_ptr->accountname, q_ptr->hostname);
#else
		if (q_ptr->inval) {
			if (q_ptr->v_unknown && is_admin(p_ptr)) strcpy(flag_str, "\377yI\377U+\377rU");
			else if (q_ptr->v_test && is_admin(p_ptr)) strcpy(flag_str, "\377yI\377U+\377oT");
			else if (q_ptr->v_outdated) strcpy(flag_str, "\377yI\377U+\377DO");
			else if (!q_ptr->v_latest && is_admin(p_ptr)) strcpy(flag_str, "\377yI\377U+\377sL");
			else strcpy(flag_str, "\377y(I)");
		} else {
			if (q_ptr->v_unknown && is_admin(p_ptr)) strcpy(flag_str, "\377r(U)");
			else if (q_ptr->v_test && is_admin(p_ptr)) strcpy(flag_str, "\377o(T)");
			else if (q_ptr->v_outdated) strcpy(flag_str, "\377D(O)");
			else if (!q_ptr->v_latest && is_admin(p_ptr)) strcpy(flag_str, "\377s(L)");
			else strcpy(flag_str, "   ");
		}
		fprintf(fff, " %s\377U (%s@%s) ", flag_str, q_ptr->accountname, q_ptr->hostname);
#endif

		/* Print location if both players are PvP-Mode */
		if (((p_ptr->mode & MODE_PVP) && (q_ptr->mode & MODE_PVP)) && !admin) {
			fprintf(fff, "%s [%d,%d]", wpos_format(-Ind, &q_ptr->wpos), q_ptr->panel_row, q_ptr->panel_col);
		}
		/* Print extra info if these people are in the same party */
		/* Hack -- always show extra info to dungeon master */
		else if ((p_ptr->party == q_ptr->party && p_ptr->party) || Ind == k || admin
#ifdef IDDC_CHAR_POSITION_INDICATOR
		    || iddc
#endif
		    ) {
			if (admin) fprintf(fff, "%s", wpos_format(Ind, &q_ptr->wpos));
			else fprintf(fff, "%s", wpos_format(-Ind, &q_ptr->wpos));

			fprintf(fff, " [%d,%d]", q_ptr->panel_row, q_ptr->panel_col);
		}

		/* Quest flag */
		fprintf(fff, " %c", (q_ptr->quest_id ? 'Q' : ' '));

		if ((!q_ptr->afk) || !strlen(q_ptr->afk_msg)) {
			if (!q_ptr->info_msg[0])
				fprintf(fff, "\n\n");
			else
				fprintf(fff, "\n     \377U(%s\377U)\n", q_ptr->info_msg);
		} else
			fprintf(fff, "\n     \377u(%s\377u)\n", q_ptr->afk_msg);

		lines += 4;

		/* hack for BIG_MAP: Screen has 42 lines for @-list, but with 4 lines
		   per entry it cuts the last entry on each page in half. Fill in 2
		   dummy lines to prevent that, for better visuals: */
		if (is_older_than(&p_ptr->version, 4, 4, 9, 4, 0, 0) && /* newer clients know div4_line */
		    big_map && lines == 40) {
			fprintf(fff, "\n\n");
			lines += 2;
		}
 } //#endif
} //#endif
	}

#ifdef TOMENET_WORLDS
	if (cfg.worldd_plist) {
		k = world_remote_players(fff);
		if (k) lines += k + 3;
	}
#endif

	/* add blank lines for more aesthetic browsing -- TODO: Make this a flag of show_file() instead */
if ((compaction == 1 || compaction == 2) /*#ifdef COMPACT_PLAYERLIST*/
    && !big_map) {
	if (is_newer_than(&p_ptr->version, 4, 4, 7, 0, 0, 0))
		lines = (((21 + HGT_PLUS) - (lines % (21 + HGT_PLUS))) % (21 + HGT_PLUS));
	else
		lines = (((20 + HGT_PLUS) - (lines % (20 + HGT_PLUS))) % (20 + HGT_PLUS));
} else if (compaction == 0 && big_map && is_newer_than(&p_ptr->version, 4, 4, 9, 3, 0, 0)) {//#else
	int div4l = 21 + HGT_PLUS - ((21 + HGT_PLUS) % 4);
	lines = (div4l - (lines % div4l)) % div4l;
} else {
	lines = (((20 + HGT_PLUS) - (lines % (20 + HGT_PLUS))) % (20 + HGT_PLUS));
}//#endif
	for (k = 1; k <= lines; k++) fprintf(fff, "\n");

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
if ((compaction == 1 || compaction == 2) /*#ifdef COMPACT_PLAYERLIST*/
    && !big_map) {
	show_file(Ind, file_name, "Players Online", line, 0, 3); //expand to divisable by 3 # of lines (which means +1)
} else if (big_map && compaction == 0) {//#else
	show_file(Ind, file_name, "Players Online", line, 0, 4); //reduce to divisable by 4 # of lines (which means -2)
} else {
	show_file(Ind, file_name, "Players Online", line, 0, 0);
}//#endif

	/* Remove the file */
	fd_kill(file_name);
}

 /*
 * Check the equipments of other player.
 */
void do_cmd_check_player_equip(int Ind, int line)
{
	int i, k;
	FILE *fff;
	char file_name[MAX_PATH_LENGTH];
	player_type *p_ptr = Players[Ind];
	bool admin = is_admin(p_ptr), init = FALSE;

	/* Temporary file */
	if (path_temp(file_name, MAX_PATH_LENGTH)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "wb");

	/* Scan the player races */
	for (k = 1; k < NumPlayers + 1; k++) {
		player_type *q_ptr = Players[k];
		byte attr = 'w';
		bool hidden = FALSE, hidden_diz = FALSE;

		/* Only print connected players */
		if (q_ptr->conn == NOT_CONNECTED)
			continue;

		/* don't display the dungeon master if the secret_dungeon_master
		 * option is set
		 */
		if (q_ptr->admin_dm && !p_ptr->admin_dm &&
		    (cfg.secret_dungeon_master)) continue;

		/*** Determine color ***/

		attr = 'G';

		/* Skip myself */
		if (Ind == k) continue;

		/* Print party members in blue */
		else if (p_ptr->party && p_ptr->party == q_ptr->party) attr = 'B';

		/* Print hostile players in red */
		else if (check_hostile(Ind, k)) attr = 'r';

		/* Print newbies/lowbies in white */
		else if (q_ptr->lev < 10) attr = 'w';

		/* Party member & hostile players only */
		/* else continue; */

		/* Only party member or those on the same dungeon level */
		//                              if ((attr != 'B') && (p_ptr->dun_depth != q_ptr->dun_depth)) continue;
		if ((attr != 'B') && (attr != 'w') && !admin) {
			/* Make sure this player is at this depth */
			if(!inarea(&p_ptr->wpos, &q_ptr->wpos)) continue;

			/* Can he see this player? */
			if (!(p_ptr->cave_flag[q_ptr->py][q_ptr->px] & CAVE_VIEW)) continue;
		}

		/* Skip invisible players */
#if 0
		if ((!p_ptr->see_inv || ((q_ptr->inventory[INVEN_OUTER].k_idx) && (q_ptr->inventory[INVEN_OUTER].tval == TV_CLOAK) && (q_ptr->inventory[INVEN_OUTER].sval == SV_SHADOW_CLOAK))) && q_ptr->invis)
		{
			if ((q_ptr->lev > p_ptr->lev) || (randint(p_ptr->lev) > (q_ptr->lev / 2)))
				continue;
		}
#endif

		/* Can see party members / newbies, even if invisible */
		if (q_ptr->invis && !admin && !((attr == 'B') || (attr == 'w')) &&
				(!p_ptr->see_inv ||
				 ((q_ptr->inventory[INVEN_OUTER].k_idx) && (q_ptr->inventory[INVEN_OUTER].tval == TV_CLOAK) && (q_ptr->inventory[INVEN_OUTER].sval == SV_SHADOW_CLOAK))) &&
				((q_ptr->lev > p_ptr->lev) || (randint(p_ptr->lev) > (q_ptr->lev / 2))))
			continue;
		if (q_ptr->cloaked == 1 && !q_ptr->cloak_neutralized && !admin && attr != 'B') continue;

		/* Add blank line for spacing */
		if (init) fprintf(fff, "\377%c\n", 'w');
		init = TRUE;

		/* Output color byte */
		fprintf(fff, "\377%c", attr);

		/* Print a message */
		do_write_others_attributes(Ind, fff, q_ptr, attr, admin);
		/* Colour might have changed due to Iron Team party name,
		   so print the closing ')' in the original colour again: */
		/* not needed anymore since we have a linebreak now
		fprintf(fff, "\377%c)", attr);*/

		fprintf(fff, "\n");

		/* Covered by a mummy wrapping? */
		if ((TOOL_EQUIPPED(q_ptr) == SV_TOOL_WRAPPING) && !admin) hidden = TRUE;

		/* Print equipments */
		for (i = (admin ? 0 : INVEN_WIELD);
#ifndef WRAPPING_NEW
		    i < (hidden ? INVEN_LEFT : INVEN_TOTAL); i++)
#else
		    i < INVEN_TOTAL; i++)
#endif
		{
			object_type *o_ptr = &q_ptr->inventory[i];
			char o_name[ONAME_LEN];
#ifdef WRAPPING_NEW
			if (hidden) {
				if ((i == INVEN_LITE || i == INVEN_AMMO) && !hidden_diz) {
					fprintf(fff, "\377%c (Covered by a grubby wrapping)\n", 'D');
					hidden_diz = TRUE;
				}
				if (i >= INVEN_LEFT && i != INVEN_LITE && i != INVEN_AMMO) continue;
			}
#endif
			if (o_ptr->tval) {
				object_desc(Ind, o_name, o_ptr, TRUE, 3 + (i < INVEN_WIELD ? 0 : 0x10));
				if (admin && i < INVEN_WIELD)
					fprintf(fff, "\377%c%c) %s\n", i < INVEN_WIELD? 'u' : 'w', 97 + i, o_name);
				else
					fprintf(fff, "\377%c %s\n", i < INVEN_WIELD? 'u' : 'w', o_name);
				hidden_diz = FALSE;
			}
		}
#ifndef WRAPPING_NEW
		/* Covered by a mummy wrapping? */
		if (hidden && !hidden_diz) {
#if 0 /* changed position of INVEN_ARM to occur before INVEN_LEFT, so following hack isn't needed anymore */
			/* for dual-wield, but also in general, INVEN_ARM should be visible too */
			object_type *o_ptr = &q_ptr->inventory[INVEN_ARM];
			char o_name[ONAME_LEN];
			if (o_ptr->tval) {
				object_desc(Ind, o_name, o_ptr, TRUE, 3 + 0x10);
				fprintf(fff, "\377w %s\n", o_name);
			}
#endif
			fprintf(fff, "\377%c (Covered by a grubby wrapping)\n", 'D');
		}
#endif
	}

       /* Close the file */
       my_fclose(fff);

       /* Display the file contents */
       show_file(Ind, file_name, "Equipment of Inspectable Players", line, 0, 0);

       /* Remove the file */
       fd_kill(file_name);
}


/*
 * List recall depths
 */
/* Allow non-admins to see starting/max level of dungeons? */
//#define SHOW_DLVL_TO_NONADMIN
void do_cmd_knowledge_dungeons(int Ind)
{
	player_type *p_ptr = Players[Ind];

//	msg_format(Ind, "The deepest point you've reached: \377G-%d\377wft", p_ptr->max_dlv * 50);

	int		i, x, y;	// num, total = 0;
	//bool	shown = FALSE;
	bool	admin = is_admin(p_ptr);
	dungeon_type *d_ptr;

	FILE *fff;

#ifdef SEPARATE_RECALL_DEPTHS
	struct worldpos wpos;
#endif

	/* Paranoia */
	// if (!letter) return;

	/* Open a new file */
	fff = my_fopen(p_ptr->infofile, "wb");

	/* Current file viewing */
	strcpy(p_ptr->cur_file, p_ptr->infofile);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;

	fprintf(fff, "\377r======== Dungeon(s) ========\n");

#ifndef SEPARATE_RECALL_DEPTHS
	fprintf(fff, "\n\377DThe deepest/highest point you've ever reached: \377s%d \377Dft (Lv \377s%d\377D)\n", p_ptr->max_dlv * 50, p_ptr->max_dlv);
#else
	fprintf(fff, "\377D(The deepest/highest point you've ever reached: \377s%d \377Dft (Lv \377s%d\377D))\n\n", p_ptr->max_dlv * 50, p_ptr->max_dlv);
 #ifndef SHOW_DLVL_TO_NONADMIN
	fprintf(fff, "\377sLocation  Dungeon/Tower Name              Your current maximum recall depth\n");
 #else
	fprintf(fff, "\377sLocation  Dungeon/Tower Name                Level        Your max recall depth\n");
 #endif
#endif

	fprintf(fff,"\n");

	for (y = 0; y < MAX_WILD_Y; y++) {
		for (x = 0; x < MAX_WILD_X; x++) {
			if (!((p_ptr->wild_map[(x + y * MAX_WILD_X) / 8] &
			    (1 << ((x + y * MAX_WILD_X) % 8))) || admin))
				continue;

			if ((d_ptr = wild_info[y][x].tower)) {
				i = d_ptr->type;
				if (i == DI_VALINOR && !admin) continue;
				fprintf(fff, " \377u(%2d,%2d)  \377w%-30s", x, y, get_dun_name(x, y, TRUE, d_ptr, 0));
#ifndef SEPARATE_RECALL_DEPTHS
				if (admin) {
					fprintf(fff, "  Lev: %3d-%3d  Req: %3d  type: %3d",
							d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
//							d_info[i].mindepth, d_info[i].mindepth + d_info[i].maxdepth - 1,
							d_info[i].min_plev, i);
 #ifdef SHOW_DLVL_TO_NONADMIN
				} else {
					fprintf(fff, "  \377sLev\377w %3d - %3d\377s",
							d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1);
 #endif
				}
#else
				wpos.wx = x; wpos.wy = y; wpos.wz = 1;
				if (admin) {
					if (p_ptr->depth_in_feet)
						fprintf(fff, "  L %3d-%3d  R %3d  t %3d  Max %6dft",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								d_info[i].min_plev, i, 50 * get_recall_depth(&wpos, p_ptr));
					else
						fprintf(fff, "  L %3d-%3d  R %3d  t %3d  Max Lv%4d",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								d_info[i].min_plev, i, get_recall_depth(&wpos, p_ptr));
				} else {
 #ifdef SHOW_DLVL_TO_NONADMIN
					if (p_ptr->depth_in_feet)
						fprintf(fff, "  %3d - %3d          %6dft",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								50 * get_recall_depth(&wpos, p_ptr));
					else
						fprintf(fff, "  %3d - %3d          Lv%4d",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								get_recall_depth(&wpos, p_ptr));
 #else
					if (p_ptr->depth_in_feet)
						fprintf(fff, "  %6dft",
								50 * get_recall_depth(&wpos, p_ptr));
					else
						fprintf(fff, "  Lv%4d",
								get_recall_depth(&wpos, p_ptr));
 #endif
				}
#endif
				fprintf(fff,"\n");
			}
			if ((d_ptr = wild_info[y][x].dungeon)) {
				i = d_ptr->type;
				if (i == DI_VALINOR && !admin) continue;
				fprintf(fff, " \377u(%2d,%2d)  \377w%-30s", x, y, get_dun_name(x, y, FALSE, d_ptr, 0));
#ifndef SEPARATE_RECALL_DEPTHS
				if (admin) {
					fprintf(fff, "  Lev: %3d-%3d  Req: %3d  type: %3d",
							d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
//							d_info[i].mindepth, d_info[i].mindepth + d_info[i].maxdepth - 1,
							d_info[i].min_plev, i);
 #ifdef SHOW_DLVL_TO_NONADMIN
				} else {
					fprintf(fff, "  \377sLev\377w %3d - %3d",
							d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1);
 #endif
				}
#else
				wpos.wx = x; wpos.wy = y; wpos.wz = -1;
				if (admin) {
					if (p_ptr->depth_in_feet)
						fprintf(fff, "  L %3d-%3d  R %3d  t %3d  Max %6dft",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								d_info[i].min_plev, i, (p_ptr->depth_in_feet ? -50 : -1) * get_recall_depth(&wpos, p_ptr));
					else
						fprintf(fff, "  L %3d-%3d  R %3d  t %3d  Max Lv%4d",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								d_info[i].min_plev, i, (p_ptr->depth_in_feet ? -50 : -1) * get_recall_depth(&wpos, p_ptr));
				} else {
 #ifdef SHOW_DLVL_TO_NONADMIN
					if (p_ptr->depth_in_feet)
						fprintf(fff, "  %3d - %3d          %6dft",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								-50 * get_recall_depth(&wpos, p_ptr));
					else
						fprintf(fff, "  %3d - %3d          Lv%4dft",
								d_ptr->baselevel, d_ptr->baselevel + d_ptr->maxdepth - 1,
								-get_recall_depth(&wpos, p_ptr));
 #else
					if (p_ptr->depth_in_feet)
						fprintf(fff, "  %6dft",
								-50 * get_recall_depth(&wpos, p_ptr));
					else
						fprintf(fff, "  Lv%4d",
								-get_recall_depth(&wpos, p_ptr));
 #endif
				}
#endif
				fprintf(fff,"\n");
			}
		}
	}

	fprintf(fff,"\n");



	fprintf(fff, "\n\n\377B======== Town(s) ========\n\n");

	/* Scan all towns */
	for (i = 0; i < numtowns; i++) {
		y = town[i].y;
		x = town[i].x;

		/* The dungeon has a valid recall depth set */
		if ((p_ptr->wild_map[(x + y * MAX_WILD_X) / 8] &
		    (1 << ((x + y * MAX_WILD_X) % 8))) || admin)
		{
			/* Describe the town locations */
			if (admin)
				fprintf(fff, " \377u(%2d,%2d)\377w %-30s  Lev: %3d", x, y,
				    town_profile[town[i].type].name, town[i].baselevel);
			else
				fprintf(fff, " \377u(%2d,%2d)\377w %-30s", x, y,
				    town_profile[town[i].type].name);

			if (p_ptr->town_x == x && p_ptr->town_y == y)
				fprintf(fff, "  \377U(default recall point)");

			fprintf(fff,"\n");
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Let the client know to expect some info */
	strcpy(p_ptr->cur_file_title, "Towns & Dungeons");
	Send_special_other(Ind);
}

/*
 * Tell players of server settings, using temporary file. - Jir -
 */
void do_cmd_check_server_settings(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		 k;

	FILE *fff;

#if 0
	char file_name[MAX_PATH_LENGTH];

	/* Temporary file */
	if (path_temp(file_name, MAX_PATH_LENGTH)) return;

	strcpy(p_ptr->infofile, file_name);
#endif

	/* Open a new file */
	fff = my_fopen(p_ptr->infofile, "wb");

	/* Current file viewing */
	strcpy(p_ptr->cur_file, p_ptr->infofile);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;


	/* Output color byte */
//	fprintf(fff, "%c", 'G');

	fprintf(fff, "%s\n", longVersion);
	fprintf(fff, "======== Server Settings ========\n\n");

	/* Output color byte */
//	fprintf(fff, "%c", 'w');

	/* General information */
	fprintf(fff, "Server notes: %s\n", cfg.server_notes);
#ifdef TEST_SERVER
	fprintf(fff, "This is a test server. Expect frequent restarts/crashes.\n");
#endif
#ifdef RPG_SERVER
	fprintf(fff, "This is an 'Ironman' server. See guide (8.5) for ruleset details.\n");
#endif
#ifdef ARCADE_SERVER
	fprintf(fff, "This is an 'Arcade' server. See guide (8.5a) for ruleset details.\n");
#endif
#ifdef FUN_SERVER
	fprintf(fff, "This is a 'Fun' server: Players may use '/wish' command freely.\n");
#endif

	fprintf(fff, "Inactive characters will be deleted after %d days.\n", CHARACTER_EXPIRY_DAYS);
	fprintf(fff, "Accounts without characters will be deleted after %d days.\n", ACCOUNT_EXPIRY_DAYS);
	fprintf(fff, "Game speed(FPS): %d (%+d%%)\n", cfg.fps, (cfg.fps-60)*100/60);
	fprintf(fff,"\n");

	fprintf(fff, "Players' running speed is boosted (x%d, ie. %+d%%).\n", cfg.running_speed, (cfg.running_speed - 5) * 100 / 5);
	fprintf(fff, "While 'resting', HP/MP recovers %d times quicker (%+d%%)\n", cfg.resting_rate, (cfg.resting_rate-3)*100/3);

	if ((k=cfg.party_xp_boost))
		fprintf(fff, "Party members get boosted exp(factor %d).\n", k);

	switch (cfg.replace_hiscore & 0x7) {
	case 0: fprintf(fff, "High-score entries are added to the high-score table.\n"); break;
	case 1: fprintf(fff, "Instead of getting added, newer score replaces older entries.\n"); break;
	case 2: fprintf(fff, "Instead of getting added, higher scores replace old entries.\n"); break;
	case 3: fprintf(fff, "Instead of getting added, higher scores replace old entries\nand one account may get a maximum of 2 scoreboard entries.\n"); break;
	case 4: fprintf(fff, "Instead of getting added, higher scores replace old entries\nand one account may get a maximum of 3 scoreboard entries.\n"); break;
	}
	if (cfg.replace_hiscore & 0x08)
		fprintf(fff, "..if ALSO the character name is the same.\n");
	if (cfg.replace_hiscore & 0x10)
		fprintf(fff, "..if ALSO the character is from same player account.\n");
	if (cfg.replace_hiscore & 0x20)
		fprintf(fff, "..if ALSO the character is of same class.\n");
	if (cfg.replace_hiscore & 0x40)
		fprintf(fff, "..if ALSO the character is of same race.\n");

	/* Several restrictions */
#if 0 /* obsolete/unused */
	if (!cfg.maximize)
		fprintf(fff, "This server is *NOT* maximized!\n");
#endif

	fprintf(fff,"\n");

	if ((k=cfg.newbies_cannot_drop))
		fprintf(fff, "Players under exp.level %d are not allowed to drop items/gold.\n", k);

	if ((k=cfg.spell_interfere))
		fprintf(fff, "Monsters adjacent to you have %d%% chance of interfering your spellcasting.\n", k);

	if ((k=cfg.spell_stack_limit))
		fprintf(fff, "Duration of assistance spells is limited to %d turns.\n", k);

	if (cfg.clone_summoning != 999)
		fprintf(fff, "Monsters may summon up to %d times until the summons start to become clones.\n", cfg.clone_summoning);

#ifdef ALLOW_NO_QUAKE_INSCRIPTION
	fprintf(fff, "You may use !E inscription on items that cause earthquakes to suppress those.\n");
#else
	fprintf(fff, "You may use !E inscription to suppress earthquakes on Grond only.\n");
#endif

	fprintf(fff,"\n");
	k=cfg.use_pk_rules;
	switch (k)
	{
		case PK_RULES_DECLARE:
			fprintf(fff, "You should use /pk first to attack other players.\n");
			break;

		case PK_RULES_NEVER:
			fprintf(fff, "You are not allowed to attack/rob other players.\n");
			break;

		case PK_RULES_TRAD:
		default:
			fprintf(fff, "You can attack/rob other players (NOT recommended!).\n");
			break;
	}

	/* level preservation */
	if (cfg.no_ghost)
		fprintf(fff, "You disappear the moment you die, without becoming a ghost.\n");
	if (cfg.lifes)
		fprintf(fff, "Players with ghosts can be resurrected up to %d times until their soul\n will escape and their bodies will be permanently destroyed.\n", cfg.lifes);
	if (cfg.houses_per_player) {
		fprintf(fff, "Players may own up to level/%d houses (caps at level 50) at once", cfg.houses_per_player);
		if (cfg.castles_per_player == 1) {
			fprintf(fff, "\n of which one may be a castle (house with moat)");
			if (cfg.castles_for_kings) fprintf(fff, "\n provided the player is a king, queen, emperor or empress.\n");
			else fprintf(fff, ".\n");
		} else if (cfg.castles_per_player) {
			fprintf(fff, "\n of which %d may be a castles (houses with moat)", cfg.castles_per_player);
			if (cfg.castles_for_kings) fprintf(fff, "\n provided the player is a king, queen, emperor or empress.\n");
			else fprintf(fff, ".\n");
		} else {
			if (cfg.castles_for_kings) fprintf(fff, "\n or castles if the player is a king, queen, emperor or empress.\n");
			else fprintf(fff, ".\n");
		}
	} else {
		fprintf(fff, "Players may own as many houses as they like");
		if (cfg.castles_per_player == 1) {
			fprintf(fff, "\n of which one may be a castle (house with moat)");
			if (cfg.castles_for_kings) fprintf(fff, "\n provided the player is a king, queen, emperor or empress.\n");
			else fprintf(fff, ".\n");
		} else if (cfg.castles_per_player) {
			fprintf(fff, "\n of which %d may be a castles (houses with moat)", cfg.castles_per_player);
			if (cfg.castles_for_kings) fprintf(fff, "\n provided the player is a king, queen, emperor or empress.\n");
			else fprintf(fff, ".\n");
		} else {
			if (cfg.castles_for_kings) fprintf(fff, "\n or castles if the player is a king, queen, emperor or empress.\n");
			else fprintf(fff, ".\n");
		}
	}

	fprintf(fff,"\n");

	if (cfg.henc_strictness) fprintf(fff, "Monster exp for non-kings is affected in the following way:\n");
	switch (cfg.henc_strictness) {
	case 4:
		fprintf(fff, "Monster exp value adjusts towards highest player on the same dungeon level.\n");
	case 3:
		fprintf(fff, "Non-sleeping monsters adjust to highest player within their awareness area.\n");
	case 2:
		fprintf(fff, "Level of a player casting support spells on you affects exp for %d turns.\n", (cfg.spell_stack_limit ? cfg.spell_stack_limit : 200));
	case 1:
		fprintf(fff, "Monsters' exp value is affected by highest attacking or targetted player.\n");
		break;
	}

	fprintf(fff,"\n");

	fprintf(fff, "The floor will be erased about %d~%d seconds after you left.\n", cfg.anti_scum, cfg.anti_scum + 10);
	if ((k=cfg.level_unstatic_chance))
		fprintf(fff, "Leaving/ghostdying in dungeon keeps the floor static for %d*dunlevel minutes.\n", k);

	if ((k=cfg.min_unstatic_level) > 0) 
		fprintf(fff, "Shallow dungeon (before level %d) will never be static. Save in town!\n", k);

	if ((k=cfg.preserve_death_level) < 201)
		fprintf(fff, "Site of death under level %d will be static, allowing others to loot it.\n", k);


	fprintf(fff,"\n");
		
        /* Items */
	if (cfg.anti_cheeze_pickup)
		fprintf(fff, "Items cannot be transferred to a character of too low a level.\n");
	if (cfg.anti_cheeze_telekinesis)
		fprintf(fff, "Items cannot be sent via telekinesis to a character of too low a level.\n");
	if (cfg.surface_item_removal) {
		fprintf(fff, "Items on the world surface will be removed after %d minutes.\n", cfg.surface_item_removal);
		fprintf(fff, "(This timeout is tripled for artifacts and unlooted unique-monster drops.)\n");
	}
	if (cfg.dungeon_item_removal) {
		fprintf(fff, "Items on a dungeon/tower floor will be removed after %d minutes.\n", cfg.dungeon_item_removal);
		fprintf(fff, "(This timeout is tripled for artifacts and unlooted unique-monster drops.)\n");
	}
	if (cfg.death_wild_item_removal)
		fprintf(fff, "Dead player's items in town will be removed after %d minutes.\n", cfg.death_wild_item_removal);
	if (cfg.long_wild_item_removal)
		fprintf(fff, "Dead player's items in wilderness will be removed after %d minutes.\n", cfg.long_wild_item_removal);

	fprintf(fff,"\n");

	/* arts & winners */
#ifdef FLUENT_ARTIFACT_RESETS
	fprintf(fff, "True artifacts will disappear some time after they have been found ('I'nspect).\n");
#else
	/*unknown, since it's done in LUA:
	fprintf(fff, "True artifacts will be reset by static schedule.");*/
#endif

	if (cfg.anti_arts_hoard)
		fprintf(fff, "True artifacts will disappear if you drop/leave them.\n");
	else {
		if (cfg.anti_arts_house)
			fprintf(fff, "True artifacts will disappear if you drop/leave them inside a house.\n");
		if (cfg.anti_arts_wild)
			fprintf(fff, "True artifacts will disappear if they are left behind in the wild.\n");
	}
	if (cfg.anti_arts_pickup)
		fprintf(fff, "Artifacts cannot be transferred to a character of too low a level.\n");
	if (cfg.anti_arts_send)
		fprintf(fff, "Artifacts cannot be sent via telekinesis.\n");

	if ((k=cfg.retire_timer) > 0)
		fprintf(fff, "The winner will automatically retire after %d minutes.\n", k);
	else if (k == 0)
		fprintf(fff, "The game ends the moment you beat the final foe, Morgoth.\n");

	if (k !=0) {
		player_type p_dummy;
		u32b resf_all, resf_win, resf_owin, resf_howin;
		bool found = FALSE;

		if ((k=cfg.unique_respawn_time))
			fprintf(fff, "After winning the game, unique monsters will resurrect randomly.(%d)\n", k);

		if (cfg.kings_etiquette)
			fprintf(fff, "The winner is not allowed to carry/use static artifacts (save Grond/Crown).\n");

		if (cfg.fallenkings_etiquette)
			fprintf(fff, "Fallen winners are not allowed to carry/use static artifacts.\n");

		p_dummy.lev = 1;
		p_dummy.total_winner = p_dummy.once_winner = TRUE;
		resf_win = make_resf(&p_dummy);

		p_dummy.total_winner = FALSE;
		resf_owin = make_resf(&p_dummy);

		p_dummy.lev = 50;
		resf_howin = make_resf(&p_dummy);

		p_dummy.once_winner = FALSE;
		resf_all = make_resf(&p_dummy);

		fprintf(fff, "WINNERS_ONLY items are findable by: ");
		if (resf_all & RESF_WINNER) {
			fprintf(fff, "Everyone");
			found = TRUE;
		} else {
			if (resf_win & RESF_WINNER) {
				fprintf(fff, "Winners");
				found = TRUE;
			}
			if (resf_owin & RESF_WINNER) {
				if (found) fprintf(fff, ", ");
				fprintf(fff, "Fallen Winners");
				found = TRUE;
			} else if (resf_howin & RESF_WINNER) {
				if (found) fprintf(fff, ", ");
				fprintf(fff, "Fallen Winners of level 50+");
				found = TRUE;
			}
		}
		if (found) fprintf(fff, ".\n");
		else fprintf(fff, "Noone.\n");

		fprintf(fff, "WINNERS_ONLY items are usable by: ");
#ifdef FALLEN_WINNERSONLY
		fprintf(fff, "Winners and fallen winners");
#else
		fprintf(fff, "Winners only");
#endif
		fprintf(fff, ".\n");

		fprintf(fff, "+LIFE randarts are findable/usable by: ");
		if (resf_all & RESF_LIFE) {
			fprintf(fff, "Everyone");
			found = TRUE;
		} else {
			if (resf_win & RESF_LIFE) {
				fprintf(fff, "Winners");
				found = TRUE;
			}
			if (resf_owin & RESF_LIFE) {
				if (found) fprintf(fff, ", ");
				fprintf(fff, "Fallen Winners");
				found = TRUE;
			} else if (resf_howin & RESF_LIFE) {
				if (found) fprintf(fff, ", ");
				fprintf(fff, "Fallen Winners of level 50+");
				found = TRUE;
			}
		}
		if (found) fprintf(fff, ".\n");
		else fprintf(fff, "Noone.\n");
	}

	fprintf(fff,"\n");

	/* monster-sets */
	fprintf(fff, "Monsters:\n");
	if (is_admin(p_ptr))
	{
		if (cfg.vanilla_monsters)
			fprintf(fff, "  Vanilla-angband(default) monsters (%d%%)\n", cfg.vanilla_monsters);
		if (cfg.zang_monsters)
			fprintf(fff, "  Zelasny Angband additions (%d%%)\n", cfg.zang_monsters);
		if (cfg.pern_monsters)
			fprintf(fff, "  Pern additions (%d%%)\n", cfg.pern_monsters);
		if (cfg.cth_monsters)
			fprintf(fff, "  Lovecraft additions (%d%%)\n", cfg.cth_monsters);
		if (cfg.cblue_monsters)
			fprintf(fff, "  C. Blue-monsters (%d%%)\n", cfg.cblue_monsters);
		if (cfg.joke_monsters)
			fprintf(fff, "  Joke-monsters (%d%%)\n", cfg.joke_monsters);
		if (cfg.pet_monsters)
			fprintf(fff, "  Pet/neutral monsters (%d%%)\n", cfg.pet_monsters);
	}
	else
	{
		if (cfg.vanilla_monsters > TELL_MONSTER_ABOVE)
			fprintf(fff, "  Vanilla-angband(default) monsters\n");
		if (cfg.zang_monsters > TELL_MONSTER_ABOVE)
			fprintf(fff, "  Zelasny Angband additions\n");
		if (cfg.pern_monsters > TELL_MONSTER_ABOVE)
			fprintf(fff, "  Pern additions\n");
		if (cfg.cth_monsters > TELL_MONSTER_ABOVE)
			fprintf(fff, "  Lovecraft additions\n");
		if (cfg.cblue_monsters > TELL_MONSTER_ABOVE)
			fprintf(fff, "  C. Blue-monsters\n");
		if (cfg.joke_monsters > TELL_MONSTER_ABOVE)
			fprintf(fff, "  Joke-monsters\n");
	}

	fprintf(fff,"\n");

	/* trivial */
	if (cfg.public_rfe)
//		fprintf(fff, "You can see RFE files via '&62' command.\n");
		fprintf(fff, "You can see RFE files via '~e' command.\n");

	/* TODO: reflect client options too */
	if (cfg.door_bump_open & BUMP_OPEN_DOOR)
//		fprintf(fff, "You'll try to open a door by bumping onto it.\n");
		fprintf(fff, "easy_open is allowed.\n");
	else
		fprintf(fff, "You should use 'o' command explicitly to open a door.\n");

	if (cfg.door_bump_open & BUMP_OPEN_TRAP)
//		fprintf(fff, "You'll try to disarm a visible trap by stepping onto it.\n");
		fprintf(fff, "easy_disarm is allowed.\n");

	if (cfg.door_bump_open & BUMP_OPEN_HOUSE)
		fprintf(fff, "You can 'walk through' your house door.\n");


	/* Administrative */
	if (is_admin(p_ptr))
	{
		/* Output color byte */
//		fprintf(fff, "%c\n", 'o');

		fprintf(fff,"\n");
		

		fprintf(fff, "==== Administrative or hidden settings ====\n");

		/* Output color byte */
//		fprintf(fff, "%c\n", 'w');

		fprintf(fff, "dun_unusual: %d (default = 200)\n", cfg.dun_unusual);
		fprintf(fff, "Stores change their inventory every %d seconds(store_turns=%d).\n", cfg.store_turns * 10 / cfg.fps, cfg.store_turns);

		fprintf(fff, "starting town: location [%d, %d], baselevel(%d)\n", cfg.town_x, cfg.town_y, cfg.town_base);
		fprintf(fff, "Bree dungeon: baselevel(%d) depth(%d)\n", cfg.dun_base, cfg.dun_max);

		if (cfg.auto_purge)
			fprintf(fff, "Non-used monsters/objects are purged every 24H.\n");

#if 0
		if (cfg.mage_hp_bonus)
			fprintf(fff, "mage_hp_bonus is applied.\n");
#endif	// 0
		if (cfg.report_to_meta)
			fprintf(fff, "Reporting to the meta-server.\n");
		if (cfg.secret_dungeon_master)
			fprintf(fff, "Dungeon Master is hidden.\n");
		else
			fprintf(fff, "Dungeon Master is *SHOWN*!!\n");
		//	cfg.unique_max_respawn_time
		//	cfg.game_port
		//	cfg.console_port
	}

	/* Close the file */
	my_fclose(fff);

	/* Let the client know to expect some info */
	strcpy(p_ptr->cur_file_title, "Server Settings");
	Send_special_other(Ind);
}

/*
 * Tell players of the # of monsters killed, using temporary file. - Jir -
 */
void do_cmd_show_monster_killed_letter(int Ind, char *letter)
{
	player_type *p_ptr = Players[Ind];

	int		i, j, num, total = 0;
	monster_race	*r_ptr;
	bool	shown = FALSE, all = FALSE;
	byte	mimic = (get_skill_scale(p_ptr, SKILL_MIMIC, 100));
//	bool	admin = is_admin(p_ptr);
	bool	druid_form, vampire_form, uniq;

	FILE *fff;

	/* Paranoia */
	// if (!letter) return;

	/* Open a new file */
	fff = my_fopen(p_ptr->infofile, "wb");

	/* Current file viewing */
	strcpy(p_ptr->cur_file, p_ptr->infofile);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;


	/* Output color byte */
	fprintf(fff, "\377D");

	if (letter && *letter) fprintf(fff, "======== Killed List for Monster Group '%c' ========\n", *letter);
	else
	{
		all = TRUE;
		fprintf(fff, "======== Killed List ========\n");
	}

	/* for each monster race */
	/* XXX I'm not sure if this list should be sorted.. */
	for (i = 1; i <= MAX_R_IDX; i++)
	{
		r_ptr = &r_info[i];
//		if (letter && *letter != r_ptr->d_char) continue;
		if (!all && !strchr(letter, r_ptr->d_char)) continue;
		num = p_ptr->r_killed[i];

		/* Hack for druid */
		druid_form = FALSE;
		if ((p_ptr->pclass == CLASS_DRUID) && mimic_druid(i, p_ptr->lev))
			druid_form = TRUE;

		/* Hack for vampires */
		vampire_form = FALSE;
		if ((p_ptr->prace == RACE_VAMPIRE) && mimic_vampire(i, p_ptr->lev))
			vampire_form = TRUE;

		/* Hack -- always show townie */
		// if (num < 1 && r_ptr->level) continue;

		if ((num < 1) && !druid_form && !vampire_form) continue;
		if (!r_ptr->name) continue;

		/* Let's not show uniques here */
		uniq = FALSE;
		if (r_ptr->flags1 & RF1_UNIQUE) {
#if 0 /* don't show uniques */
			continue;
#else /* show uniques */
			/* only show uniques we killed ourselves */
			if (num != 1) continue;
			uniq = TRUE;
#endif
		}


		if (!uniq) fprintf(fff, "\377s(%4d) \377%c%c\377s  ", i, color_attr_to_char(r_ptr->d_attr), r_ptr->d_char); /* mimics need that number for Polymorph Self Into.. */
		else fprintf(fff, "       \377%c%c\377s  ", color_attr_to_char(r_ptr->d_attr), r_ptr->d_char);

		if (uniq) {
			fprintf(fff, "\377U%-30s\n", r_name + r_ptr->name);
		}
		else if (((mimic && (mimic >= r_ptr->level)) || druid_form) &&
		    !((p_ptr->pclass == CLASS_DRUID) && !mimic_druid(i, p_ptr->lev)) &&
		    !((p_ptr->prace == RACE_VAMPIRE) && !mimic_vampire(i, p_ptr->lev)) &&
		    !(p_ptr->pclass == CLASS_SHAMAN && !mimic_shaman(i)))
		{
			j = r_ptr->level - num;

			if ((j > 0) && !druid_form && !vampire_form)
				fprintf(fff, "\377w%-30s : %4d slain  (%d more to go)\n",
						r_name + r_ptr->name, num, j);
			else {
				if (p_ptr->body_monster == i)
					fprintf(fff, "\377B%-30s : %4d slain  ** Your current form **\n",
							r_name + r_ptr->name, num);
				else fprintf(fff, "\377G%-30s : %4d slain  (learnt)\n",
						r_name + r_ptr->name, num);
			}
		} else {
			fprintf(fff, "\377w%-30s : %4d slain\n", r_name + r_ptr->name, num);
		}
		total += num;
		shown = TRUE;
	}

	if (!shown) fprintf(fff, "Nothing so far.\n");
	else fprintf(fff, "\nTotal : %d\n", total);

	/* Close the file */
	my_fclose(fff);

	/* Let the client know to expect some info */
	strcpy(p_ptr->cur_file_title, "Monster Information");
	Send_special_other(Ind);
}


/* Tell the player of her/his houses.	- Jir - */
/* TODO: handle HF_DELETED */
void do_cmd_show_houses(int Ind)
{
	player_type *p_ptr = Players[Ind];
	house_type *h_ptr;
	struct dna_type *dna;
	cptr name;

	int		i, total = 0;	//j, num,
	bool	shown = FALSE;
	bool	admin = is_admin(p_ptr);

	FILE *fff;

	/* Paranoia */
	// if (!letter) return;

	/* Open a new file */
	fff = my_fopen(p_ptr->infofile, "wb");

	/* Current file viewing */
	strcpy(p_ptr->cur_file, p_ptr->infofile);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;


	/* Output color byte */
//	fprintf(fff, "%c", 'G');

	if (!is_newer_than(&p_ptr->version, 4, 4, 7, 0, 0, 0))
		fprintf(fff, "======== House List ========\n");

	for(i = 0; i < num_houses; i++) {
		//if(!houses[i].dna->owner) continue;
		//if(!admin && houses[i].dna->owner != p_ptr->id) continue;
		h_ptr = &houses[i];
		dna = h_ptr->dna;

		if (!access_door(Ind, h_ptr->dna, FALSE) && !admin_p(Ind)) continue;

		shown = TRUE;
		total++;

		/* use door colour for the list entry too */
		fprintf(fff, "\377%c", color_attr_to_char((char)access_door_colour(Ind, h_ptr->dna)));

		fprintf(fff, "%3d)   [%d,%d] in %s", total,
		    h_ptr->dy * 5 / MAX_HGT, h_ptr->dx * 5 / MAX_WID,
		    wpos_format(Ind, &h_ptr->wpos));
//		    h_ptr->wpos.wz*50, h_ptr->wpos.wx, h_ptr->wpos.wy);

		if (dna->creator == p_ptr->dna) {
			/* Take player's CHR into account */
			int factor = adj_chr_gold[p_ptr->stat_ind[A_CHR]];
			int price = dna->price / 100 * factor;

			if (price < 100) price = 100;
			fprintf(fff, "  %dau", price / 2);
		}

		if (admin) {
#if 0
			name = lookup_player_name(houses[i].dna->creator);
			if (name) fprintf(fff, "  Creator:%s", name);
			else fprintf(fff, "  Dead's. ID: %d", dna->creator);
#endif	// 0
			if (dna->owner_type == OT_PLAYER) {
				name = lookup_player_name(houses[i].dna->owner);
				if (name) fprintf(fff, "  ID: %d  Owner: %s", dna->owner, name);
				else fprintf(fff, "  ID: %d", dna->owner);
			} else if (dna->owner_type == OT_GUILD) {
				name = lookup_player_name(guilds[houses[i].dna->owner].master);
				if (name) fprintf(fff, "  ID: %d  Master: %s", dna->owner, name);
				else fprintf(fff, "  ID: %d", dna->owner);
			} else { /* paranoia */
				fprintf(fff, "  ID: %d", dna->owner);
			}
		}

#if 1
		switch(dna->owner_type) {
		case OT_PLAYER:
#if 0
			if (dna->owner == dna->creator) break;
			name = lookup_player_name(dna->owner);
			if (name) fprintf(fff, "  Legal owner:%s", name);
#endif	// 0
#if 0	// nothig so far.
			else {
				s_printf("Found old player houses. ID: %d\n", houses[i].dna->owner);
				kill_houses(houses[i].dna->owner, OT_PLAYER);
			}
#endif	// 0
			break;

		case OT_PARTY:
			name = parties[dna->owner].name;
			if(strlen(name)) fprintf(fff, "  as party %s", name);
#if 0	// nothig so far.
			else {
				s_printf("Found old party houses. ID: %d\n", houses[i].dna->owner);
				kill_houses(houses[i].dna->owner, OT_PARTY);
			}
#endif	// 0
			break;
		case OT_CLASS:
			name = class_info[dna->owner].title;
			if(strlen(name)) fprintf(fff, "  as class %s", name);
			break;
		case OT_RACE:
			name = race_info[dna->owner].title;
			if(strlen(name)) fprintf(fff, "  as race %s", name);
			break;
		case OT_GUILD:
			name = guilds[dna->owner].name;
			if(strlen(name)) fprintf(fff, "  as guild %s", name);
			break;
		}
#endif	// 0

		fprintf(fff, "\n");
	}

	if (!shown) fprintf(fff, "You're homeless for now.\n");
//	else fprintf(fff, "\nTotal : %d\n", total);

	/* Close the file */
	my_fclose(fff);

	/* Let the client know to expect some info */
	strcpy(p_ptr->cur_file_title, "Houses");
	Send_special_other(Ind);
}

/*
 * Tell players of the known items, using temporary file. - Jir -
 */
/*
 * NOTE: we don't show the flavor of already-identified objects
 * since flavors are the same for all the player.
 */
void do_cmd_show_known_item_letter(int Ind, char *letter)
{
	player_type *p_ptr = Players[Ind];

	int		i, j, total = 0;
	object_kind	*k_ptr;
	object_type forge;
	char o_name[ONAME_LEN];
	bool all = FALSE;
	bool admin = is_admin(p_ptr);
	s16b idx[max_k_idx];

	FILE *fff;

	/* Paranoia */
	// if (!letter) return;

	/* Open a new file */
	fff = my_fopen(p_ptr->infofile, "wb");

	/* Current file viewing */
	strcpy(p_ptr->cur_file, p_ptr->infofile);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;


	/* Output color byte */
//	fprintf(fff, "%c", 'G');

	if (letter && *letter) fprintf(fff, "\377y======== Objects known (%c) ========\n", *letter);
	else
	{
		all = TRUE;
		fprintf(fff, "\377y======== Objects known ========\n");
	}

#if 0
	/* for each object kind */
	for (i = 1; i <= MAX_K_IDX; i++)
	{
		k_ptr = &k_info[i];
		if (!k_ptr->name) continue;
		if (!all && *letter != k_ptr->d_char) continue;	// k_char ?
//		if (!object_easy_know(i)) continue;
//		if (!k_ptr->easy_know) continue;
		if (!k_ptr->has_flavor) continue;
		if (!p_ptr->obj_aware[i]) continue;

		/* Create the object */
		invcopy(&forge, i);

		/* Describe the artifact */
		object_desc_store(Ind, o_name, &forge, FALSE, 0);

		/* Hack -- remove {0} */
		j = strlen(o_name);
		o_name[j-4] = '\0';

		if (admin) fprintf(fff, "%3d, %3d)  \377w", k_ptr->tval, k_ptr->sval);

		fprintf(fff, "%s\n", o_name);

		total++;
		shown = TRUE;
	}

	/* for each object kind */
	for (i = 1; i <= MAX_K_IDX; i++)
	{
		k_ptr = &k_info[i];
		if (!k_ptr->name) continue;
		if (letter && *letter != k_ptr->d_char) continue;	// k_char ?
//		if (!object_easy_know(i)) continue;
//		if (!k_ptr->easy_know) continue;
		if (!k_ptr->has_flavor) continue;
		if (p_ptr->obj_aware[i]) continue;
		if (!p_ptr->obj_tried[i]) continue;

		/* Create the object */
		invcopy(&forge, i);

		/* Describe the artifact */
		object_desc(Ind, o_name, &forge, FALSE, 0);

		/* Hack -- remove {0} */
		j = strlen(o_name);
		o_name[j-4] = '\0';

		if (admin) fprintf(fff, "%3d, %3d)  \377w", k_ptr->tval, k_ptr->sval);

		fprintf(fff, "%s\n", o_name);

		total++;
		shown = TRUE;
	}
#endif	// 0

	/* for each object kind */
	for (i = 1; i < max_k_idx; i++)
	{
		k_ptr = &k_info[i];
		if (!k_ptr->name) continue;
		if (!all && *letter != k_ptr->d_char) continue;	// k_char ?
//		if (!object_easy_know(i)) continue;
//		if (!k_ptr->easy_know) continue;
		if (!k_ptr->has_flavor) continue;
		if (!p_ptr->obj_aware[i]) continue;

		idx[total++] = i;
	}

	if (total)
	{
		/* Setup the sorter */
		ang_sort_comp = ang_sort_comp_tval;
		ang_sort_swap = ang_sort_swap_s16b;

		/* Sort the item list according to value */
		ang_sort(Ind, &idx, NULL, total);

		/* for each object kind */
		for (i = total - 1; i >= 0; i--)
		{
			k_ptr = &k_info[idx[i]];

			/* Create the object */
			invcopy(&forge, idx[i]);

			/* Describe the artifact */
			object_desc_store(Ind, o_name, &forge, FALSE, 0);

			/* Hack -- remove {0} */
			j = strlen(o_name);
			o_name[j-4] = '\0';

			if (admin) fprintf(fff, "\377s(%3d, %3d)  \377w", k_ptr->tval, k_ptr->sval);

			fprintf(fff, "%s\n", o_name);
		}
	}


	if (!total) fprintf(fff, "Nothing so far.\n");
	else fprintf(fff, "\nTotal : %d\n", total);

	fprintf(fff, "\n");

	if (!all) fprintf(fff, "\377o======== Objects tried (%c) ========\n", *letter);
	else fprintf(fff, "\377o======== Objects tried ========\n");

	total = 0;

	/* for each object kind */
	for (i = 1; i < max_k_idx; i++)
	{
		k_ptr = &k_info[i];
		if (!k_ptr->name) continue;
		if (!all && *letter != k_ptr->d_char) continue;	// k_char ?
//		if (!object_easy_know(i)) continue;
//		if (!k_ptr->easy_know) continue;
		if (!k_ptr->has_flavor) continue;
		if (p_ptr->obj_aware[i]) continue;
		if (!p_ptr->obj_tried[i]) continue;

		idx[total++] = i;
	}

	if (total)
	{
		/* Setup the sorter */
		ang_sort_comp = ang_sort_comp_tval;
		ang_sort_swap = ang_sort_swap_s16b;

		/* Sort the item list according to value */
		ang_sort(Ind, &idx, NULL, total);

		/* for each object kind */
		for (i = total - 1; i >= 0; i--)
		{
			k_ptr = &k_info[idx[i]];

			/* Create the object */
			invcopy(&forge, idx[i]);

			/* Describe the artifact */
			object_desc(Ind, o_name, &forge, FALSE, 0);

			/* Hack -- remove {0} */
			j = strlen(o_name);
			o_name[j-4] = '\0';

			if (admin) fprintf(fff, "\377s(%3d, %3d)  \377w", k_ptr->tval, k_ptr->sval);

			fprintf(fff, "%s\n", o_name);
		}
	}


//	fprintf(fff, "\n");

	if (!total) fprintf(fff, "Nothing so far.\n");
	else fprintf(fff, "\nTotal : %d\n", total);

	/* Close the file */
	my_fclose(fff);

	/* Let the client know to expect some info */
	strcpy(p_ptr->cur_file_title, "Object Information");
	Send_special_other(Ind);
}

/*
 * Check the status of traps
 */
void do_cmd_knowledge_traps(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int k;

	FILE *fff;

	trap_kind *t_ptr;

	int	total = 0;
	bool shown = FALSE;
	bool admin = is_admin(p_ptr);

	/* Open a new file */
	fff = my_fopen(p_ptr->infofile, "wb");

	/* Current file viewing */
	strcpy(p_ptr->cur_file, p_ptr->infofile);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;

	fprintf(fff, "\377s======== known traps ========\n");

	/* Scan the traps */
	for (k = 0; k < MAX_T_IDX; k++)
	{
		/* Get the trap */
		t_ptr = &t_info[k];

		/* Skip "empty" traps */
		if (!t_ptr->name) continue;

		/* Skip unidentified traps */
		if(!p_ptr->trap_ident[k]) continue;

		if (admin) fprintf(fff, "(%3d)", k);

		/* Hack -- Build the trap name */
		fprintf(fff, "     %s\n", t_name + t_ptr->name);

		total++;
		shown = TRUE;
	}

	fprintf(fff, "\n");

	if (!shown) fprintf(fff, "Nothing so far.\n");
	else fprintf(fff, "\nTotal : %d\n", total);

	/* Close the file */
	my_fclose(fff);

	/* Let the client know to expect some info */
	strcpy(p_ptr->cur_file_title, "Trap Information");
	Send_special_other(Ind);
}

/*
 * Display motd, same as /motd command
 */
void show_motd2(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int k;
	bool shown = FALSE;
	FILE *fff;

	/* Open a new file */
	fff = my_fopen(p_ptr->infofile, "wb");

	/* Current file viewing */
	strcpy(p_ptr->cur_file, p_ptr->infofile);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;

	/* Scan the lines */
	for (k = 0; k < MAX_ADMINNOTES; k++) {
		if (!strcmp(admin_note[k], "")) continue;
		fprintf(fff, "\377sMotD: %s\n", admin_note[k]);
		shown = TRUE;
	}
	fprintf(fff, "\n");

	if (!shown) fprintf(fff, "No message of the day has been set.\n");

	/* Close the file */
	my_fclose(fff);

	/* Let the client know to expect some info */
	strcpy(p_ptr->cur_file_title, "Message of the Day");
	Send_special_other(Ind);
}

/*
 * Display the time and date
 */
static void fetch_time_diz(char *path, char *desc) {
	FILE *fff;
	char buf[1024];

	int start = 9999;
	int end = -9999;
	int num = 0;

	int hour = bst(HOUR, turn);
	int min = bst(MINUTE, turn);
	int full = hour * 100 + min;

	/* Open this file */
	fff = my_fopen(path, "r");

	/* Oops */
	if (!fff) return;

	/* Find this time */
	while (!my_fgets(fff, buf, 1024, FALSE)) {
		/* Ignore comments */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Ignore invalid lines */
		if (buf[1] != ':') continue;

		/* Process 'Start' */
		if (buf[0] == 'S') {
			/* Extract the starting time */
			start = atoi(buf + 2);

			/* Assume valid for an hour */
			end = start + 59;

			/* Next... */
			continue;
		}

		/* Process 'End' */
		if (buf[0] == 'E') {
			/* Extract the ending time */
			end = atoi(buf + 2);

			/* Next... */
			continue;
		}

		/* Ignore incorrect range */
		if ((start > full) || (full > end)) continue;

		/* Process 'Description' */
		if (buf[0] == 'D') {
			num++;

			/* Apply the randomizer */
			if (!rand_int(num)) strcpy(desc, buf + 2);

			/* Next... */
			continue;
		}
	}

	/* Close the file */
	my_fclose(fff);
}
void do_cmd_time(int Ind)
{
	player_type *p_ptr = Players[Ind];
	bool fun = FALSE;

	int day = bst(DAY, turn);
	int hour = bst(HOUR, turn);
	int min = bst(MINUTE, turn);

	char buf2[20];
	char desc[1024];
	char buf[1024];
	char desc2[1024];

	/* Format time of the day */
	strnfmt(buf2, 20, get_day(bst(YEAR, turn))); /* hack: abuse get_day()'s capabilities */

	/* Display current date in the Elvish calendar */
	msg_format(Ind, "This is %s of the %s year of the third age.",
	           get_month_name(day, is_admin(p_ptr), FALSE), buf2);

	/* Message */
	desc[0] = 0;
	sprintf(desc2, "The time is %d:%02d %s. ",
	     (hour % 12 == 0) ? 12 : (hour % 12),
	     min, (hour < 12) ? "AM" : "PM");

#if CHATTERBOX_LEVEL > 2
	/* Find the path */
	if (!rand_int(10) || p_ptr->image) fun = TRUE;
	if (fun) path_build(buf, 1024, ANGBAND_DIR_TEXT, "timefun.txt");
	else path_build(buf, 1024, ANGBAND_DIR_TEXT, "timenorm.txt");

	/* try to find a fitting description */
	fetch_time_diz(buf, desc);
	/* found none? */
	if (!desc[0]) {
		/* if we were looking for silyl descriptions, try for a serious one instead */
		if (fun) {
			path_build(buf, 1024, ANGBAND_DIR_TEXT, "timenorm.txt");
			fetch_time_diz(buf, desc);
		}
		/* give up */
		if (!desc[0]) strcpy(desc, "It is a strange time.");
	}

	/* Message */
	strcat(desc2, desc);
	msg_print(Ind, desc2);

#endif	// 0
}

/*
 * Prepare to view already-existing text file. full path is needed.
 * do_cmd_check_other is called after the client is ready.	- Jir -
 *
 * Unlike show_file and do_cmd_help_aux, this can display the file
 * w/o request from client, ie. no new packet definition etc. is needed.
 */
void do_cmd_check_other_prepare(int Ind, char *path, char *title)
{
	player_type *p_ptr = Players[Ind];

	/* Current file viewing */
	strcpy(p_ptr->cur_file, path);
	strcpy(p_ptr->cur_file_title, title);

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;

	/* Let the client know to expect some info */
	Send_special_other(Ind);
}


/*
 * Scroll through *ID* or Self Knowledge information.
 */
//void do_cmd_check_other(int Ind, int line, int color)
void do_cmd_check_other(int Ind, s32b line)
{
	player_type *p_ptr = Players[Ind];


	/* Make sure the player is allowed to */
	if (!p_ptr->special_file_type) return;

	/* Display the file contents */
	if (p_ptr->cur_file_title[0])
		show_file(Ind, p_ptr->cur_file, p_ptr->cur_file_title, line, 0, 0);
	else
		show_file(Ind, p_ptr->cur_file, "Information", line, 0, 0);
//	show_file(Ind, p_ptr->cur_file, "Extra Info", line, color, 0);

#if 0
	/* Remove the file */
	fd_kill(p_ptr->infofile);

	strcpy(p_ptr->infofile, "");
#endif	// 0
}

#if 0
void do_cmd_check_other(int Ind, s32b line)
{
	player_type *p_ptr = Players[Ind];

	int n = 0;

	FILE *fff;

	char file_name[MAX_PATH_LENGTH];


	/* Make sure the player is allowed to */
	if (!p_ptr->special_file_type) return;

	/* Temporary file */
	if (path_temp(file_name, MAX_PATH_LENGTH)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "wb");

	/* Scan "info" */
	while (n < 128 && p_ptr->info[n] && strlen(p_ptr->info[n]))
	{
		/* Dump a line of info */
		fprintf(fff, p_ptr->info[n]);

		/* Newline */
		fprintf(fff, "\n");

		/* Next line */
		n++;
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(Ind, file_name, "Extra Info", line, 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}
#endif	// 0
