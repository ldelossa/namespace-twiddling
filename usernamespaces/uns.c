#include <linux/sched.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "./lib/str.h"

static const char *uid_map_fmt = "/proc/%i/uid_map";
static const char *gid_map_fmt = "/proc/%i/gid_map";
static const char *mapping = "0 %i 1\n";

// user_ns holds the fields necessary to create and map
// a user_namespace.
typedef struct user_ns_data {
    pid_t pid;
    uid_t uid;
    gid_t gid;
    char *g_mapping;
    char *u_mapping;
    char *uid_map;
    char *gid_map;
} user_ns_data;

static user_ns_data* init_user_ns() {
    user_ns_data *ns = malloc(sizeof(user_ns_data));
    if (!ns) {
        return ns;
    }

    ns->pid = getpid();
    ns->uid = getuid();
    ns->gid = getgid();

    size_t buflen;

    ns->uid_map = FMT_SIZED_BUFFER(uid_map_fmt, ns->pid);
    if (!ns->uid_map) {
        goto error_exit;
    }
    sprintf(ns->uid_map, uid_map_fmt, ns->pid);

    ns->gid_map = FMT_SIZED_BUFFER(gid_map_fmt, ns->pid);
    if (!ns->gid_map) {
        goto error_exit;
    }
    sprintf(ns->gid_map, gid_map_fmt, ns->pid);

    ns->u_mapping = FMT_SIZED_BUFFER(mapping, ns->uid);
    if (!ns->u_mapping) {
        goto error_exit;
    }
    sprintf(ns->u_mapping, mapping, ns->uid);

    ns->g_mapping = FMT_SIZED_BUFFER(mapping, ns->gid);
    if (!ns->g_mapping) {
        goto error_exit;
    }
    sprintf(ns->g_mapping, mapping, ns->gid);

    return ns;

    error_exit:
    if (ns->uid_map) {
        free(ns->uid_map);
    }  
    if (ns->gid_map) {
        free(ns->gid_map);
    }  
    if (ns->u_mapping) {
        free(ns->u_mapping);
    }  
    if (ns->u_mapping) {
        free(ns->u_mapping);
    }  
    free(ns);
    return NULL;
}

static void free_user_ns(user_ns_data *ns) {
    free(ns->g_mapping);
    free(ns->u_mapping);
    free(ns->uid_map);
    free(ns->gid_map);
    free(ns);
}


// write the configured mapping to the discovered uid_map
// and gid_map files for this pid.
//
// should be called only with an init'd user_ns_data
// structure. 
static int8_t write_map(char map[], char mapping[]) {
    FILE *fp;
    fp = fopen(map, "w");
    if (!fp) {
        return -1;
    }
    if(fprintf(fp, mapping) < 0) {
        return -1;
    }
    fclose(fp);
    return 0;
}

static int8_t read_map(char map[]) {
    FILE *fp;
    fp = fopen(map, "r");
    if (!fp) {
        return -1;
    }

    char buf[256];
    while (fgets(buf, 256, fp)) {
        puts(buf); 
    }

    if (!feof(fp)) {
        return -1;
    }
    return 1;
}

extern int unshare(int flags);

int main(int argc, char *argv[]) {
    printf("%d\n", getuid());
    user_ns_data *d = init_user_ns();
    if (!d) {
        return 1;
    }

    const int flag = 0 | CLONE_NEWUSER;
    if (unshare(flag) < 0) {
        perror("Failed unshare");
        return EXIT_FAILURE;
    }
    printf("%d\n", getuid());
    
    if (write_map(d->uid_map, d->u_mapping) < 0) {
        perror("Failed uid_map write");
        return EXIT_FAILURE;
    }

    if (write_map(d->gid_map, d->g_mapping) < 0) {
        perror("Failed gid_map write");
        return EXIT_FAILURE;
    }

    if (write_map(d->uid_map, d->u_mapping) < 0) {
        perror("Failed uid_map write");
        return EXIT_FAILURE;
    }

    printf("%d", getuid());
}
