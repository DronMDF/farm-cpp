require "mkmf"

$LDFLAGS << " -pthread -lcrypto"

create_makefile "score_index/score_index"
