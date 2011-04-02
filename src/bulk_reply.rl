%%{
    machine bulk_reply;

    include common "common.rl";

    bulk_reply = chunk @finalize;

}%%

