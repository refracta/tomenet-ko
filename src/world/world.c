/* tomenet world main server - copyright 2002 evileye
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "world.h"

#define MAX(x,y) (x > y ? x : y)

extern int bpipe;

struct secure secure;
struct serverinfo slist[MAX_SERVERS];
int snum=0;

void handle(struct client *ccl);
void relay(struct wpacket *wpk, struct client *talker);
void wproto(struct client *ccl);
void addclient(int fd);
struct client *remclient(struct client *dcl);

struct client *clist=NULL;

void world(int ser){
	int sl;
	struct sockaddr_in cl_in;
	int length=sizeof(struct sockaddr_in);
	char buff[40], *check;
	fd_set fds;
/*	secure.secure=1;	*//* default security setting */
	secure.secure=0;
	secure.chat=1;

	while(1){
		int mfd=ser;
		struct client *c_cl;

		FD_ZERO(&fds);
		FD_SET(ser, &fds);

		for(c_cl=clist; c_cl; c_cl=c_cl->next){
			mfd=MAX(mfd, c_cl->fd);
			FD_SET(c_cl->fd, &fds);
		}

		sl=select(mfd+1, &fds, NULL, NULL, NULL);
		if(sl==-1){
                        fprintf(stderr, "select broke\n");
			return;
		}
		if(FD_ISSET(ser, &fds)){
			sl=accept(ser, (struct sockaddr*)&cl_in, &length);
			if(sl==-1){
				fprintf(stderr, "accept broke\n");
				return;
			}
#if 0
			check=(char*)inet_ntop(AF_INET, &cl_in.sin_addr, &buff, 40);
			if(check){
				if(cl_in.sin_len){
					printf("accepted connect from %s\n", buff);
				}
			}
			else{
				fprintf(stderr, "Got connection. unable to display remote host. errno %d\n", errno);
			}
#endif
			addclient(sl);
                        fprintf(stderr, "added!\n");
		}

		for(c_cl=clist; c_cl; c_cl=c_cl->next)
			if(FD_ISSET(c_cl->fd, &fds)) handle(c_cl);
		c_cl=clist;
		while(c_cl){
			if(c_cl->flags & CL_QUIT || (c_cl->authed==-1 && secure.secure))
				c_cl=remclient(c_cl);
			else c_cl=c_cl->next;
		}
	}
}

void handle(struct client *ccl){
	int x;
        fprintf(stderr, "handling\n");
        x=recv(ccl->fd, ccl->buf+ccl->blen, 1024-ccl->blen, 0);

	/* Error condition */
	if(x==-1){
		fprintf(stderr, "Error. killing client %d\n", errno);
		ccl->flags|=CL_QUIT;
		return;
	}

	/* Connection death most likely */
	if(x==0){
		fprintf(stderr, "Client quit %d\n", errno);
		ccl->flags|=CL_QUIT;
		return;
	}
	ccl->blen+=x;
	wproto(ccl);
}

void wproto(struct client *ccl){
	struct wpacket *wpk=(struct wpacket*)ccl->buf;
        while(ccl->blen>=sizeof(struct wpacket)){
                fprintf(stderr, "protoing... type %d\n", wpk->type);
                switch(wpk->type){
			case WP_LACCOUNT:
				/* ignore unauthed servers
				   only legitimate servers should
				   ever send this */
				if(ccl->authed>0){
					l_account(wpk, ccl);
				}
				break;
			case WP_RESTART:
				/* mass restart */
				if(ccl->authed>0){
					relay(wpk, ccl);
				}
				break;
			case WP_AUTH:
			/* method - plaintext password on server.
			   server uses this to encrypt an answer to a RANDOMLY
			   generated plaintext password made here.
			   without this anyone can hack,
			   making fake messages etc.
			   It will be done BEFORE savefiles
			   and other data is shared. Some machines may
			   use a dynamic IP, so this is made *more* necessary */

				ccl->authed=pwcheck(wpk->d.auth.pass, wpk->d.auth.val);
				break;
			case WP_CHAT:
                                /* only relay all for now */
				if(ccl->authed && ((ccl->authed>0) || secure.chat)){
					char msg[160];
					sprintf(msg, "\377o[\377%c%d\377o] %s", (ccl->authed>0 ? 'g' : 'r'), ccl->authed, wpk->d.chat.ctxt);
					strncpy(wpk->d.chat.ctxt, msg, 120);
					relay(wpk, ccl);
				}
				break;
			case WP_PMSG:
				/* MUST be authed for private messages */
				if(ccl->authed>0){
					struct client *dcl;
					for(dcl=clist; dcl; dcl=dcl->next){
						if(dcl->authed==wpk->d.pmsg.sid){
							send(dcl->fd, wpk, sizeof(struct wpacket), 0); 
						}
					}
				}
				break;
			case WP_NPLAYER:
			case WP_QPLAYER:
				/* STORE players here */
				if(ccl->authed && (ccl->authed>0 || secure.play)){
					wpk->d.play.server=ccl->authed;
					add_rplayer(wpk);
					relay(wpk, ccl);
				}
				break;
			case WP_MESSAGE:
				/* simple relay message */
				if(ccl->authed && (ccl->authed>0 || secure.msgs)){
					relay(wpk, ccl);
				}
				break;
			case WP_LOCK:
				/* lock server object */
				attempt_lock(ccl, wpk->d.lock.ltype, wpk->d.lock.ttl, wpk->d.lock.obj);
				break;
			case WP_UNLOCK:
				/* unlock server object */
				attempt_unlock(ccl, wpk->d.lock.ltype, wpk->d.lock.ttl, wpk->d.lock.obj);
				break;
			default:
				fprintf(stderr, "ignoring undefined packet %d\n", wpk->type);
		}
		if(ccl->blen>sizeof(struct wpacket)){
			memcpy(ccl->buf, ccl->buf+sizeof(struct wpacket), ccl->blen-sizeof(struct wpacket));
		}
		ccl->blen-=sizeof(struct wpacket);
	}
}

/* Send duplicate packet to all servers except originating
   one */
void relay(struct wpacket *wpk, struct client *talker){
	struct client *ccl;
	for(ccl=clist; ccl; ccl=ccl->next){
		if(ccl!=talker){
			send(ccl->fd, wpk, sizeof(struct wpacket), 0); 
			/* Temporary stderr output */
			if(bpipe){
				fprintf(stderr, "SIGPIPE from relay (fd: %d)\n", ccl->fd);
				bpipe=0;
			}
		}
	}
}

void reply(struct wpacket *wpk, struct client *ccl){
	send(ccl->fd, wpk, sizeof(struct wpacket), 0);
	if(bpipe){
		fprintf(stderr, "SIGPIPE from reply (fd: %d)\n", ccl->fd);
		bpipe=0;
	}
}

/* add a new client to the server's list */
void addclient(int fd){
	struct client *ncl;
	ncl=malloc(sizeof(struct client));
	if(ncl){
		memset(ncl, 0, sizeof(struct client));
		ncl->fd=fd;
		ncl->next=clist;
		ncl->authed=0;
		initauth(ncl);
		send_rplay(ncl);
		clist=ncl;
	}
}

struct client *remclient(struct client *dcl){
	struct client *ccl;
	ccl=clist;
	if(dcl->authed>0){
		/* Tell other servers if an authed server goes down */
		struct wpacket spk;
		rem_players(dcl->authed);
		spk.type=WP_SQUIT;
		spk.d.sid=dcl->authed;
		relay(&spk, dcl);
	}
	if(dcl==clist){
		clist=ccl->next;
		close(dcl->fd);
		free(dcl);
		return(clist);
	}
	while(ccl){
		if(ccl->next==dcl){
			ccl->next=dcl->next;
			close(dcl->fd);
			free(dcl);
			return(ccl->next);
		}
		ccl=ccl->next;
	}
	fprintf(stderr, "Unable to remove connection\n");
	return(clist);
}
