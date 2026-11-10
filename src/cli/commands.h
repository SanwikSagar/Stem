#ifndef STEM_CLI_COMMANDS_H
#define STEM_CLI_COMMANDS_H

/* Command implementations */
int cmd_init(int argc, char **argv);
int cmd_clone(int argc, char **argv);
int cmd_add(int argc, char **argv);
int cmd_commit(int argc, char **argv);
int cmd_push(int argc, char **argv);
int cmd_pull(int argc, char **argv);
int cmd_sync(int argc, char **argv);
int cmd_status(int argc, char **argv);
int cmd_history(int argc, char **argv);
int cmd_diff(int argc, char **argv);
int cmd_branch(int argc, char **argv);
int cmd_switch(int argc, char **argv);
int cmd_merge(int argc, char **argv);
int cmd_tag(int argc, char **argv);
int cmd_restore(int argc, char **argv);
int cmd_verify(int argc, char **argv);
int cmd_doctor(int argc, char **argv);
int cmd_remote(int argc, char **argv);
int cmd_config(int argc, char **argv);

#endif /* STEM_CLI_COMMANDS_H */
