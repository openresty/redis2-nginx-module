%%{
    machine single_line_reply;

    include common "common.rl";

    single_line_reply = [:\+\-] (any* -- CRLF) CRLF @finalize;
}%%

