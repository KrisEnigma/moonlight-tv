#!/usr/bin/awk -f
# Convert gettext .po (subset) to JSON for webOS resBundle (cstrings.json).

function po_finalize(s) {
    gsub(/\\n/, "\n", s)
    gsub(/\\t/, "\t", s)
    gsub(/\\r/, "\r", s)
    gsub(/\\"/, "\"", s)
    gsub(/\\\\/, "\\", s)
    return s
}

function json_escape(s,    i, c, out) {
    out = ""
    for (i = 1; i <= length(s); i++) {
        c = substr(s, i, 1)
        if (c == "\\") {
            out = out "\\\\"
        } else if (c == "\"") {
            out = out "\\\""
        } else if (c == "\n") {
            out = out "\\n"
        } else if (c == "\r") {
            out = out "\\r"
        } else if (c == "\t") {
            out = out "\\t"
        } else {
            out = out c
        }
    }
    return out
}

function emit(msgid, msgstr) {
    msgid = po_finalize(msgid)
    msgstr = po_finalize(msgstr)
    if (msgid == "" || msgstr == "") {
        return
    }
    if (entry_count > 0) {
        printf(",\n")
    }
    printf("  \"%s\": \"%s\"", json_escape(msgid), json_escape(msgstr))
    entry_count++
}

function reset_entry() {
    mode = 0
    cur_id = ""
    buf = ""
}

BEGIN {
    mode = 0
    entry_count = 0
    print("{")
}

/^msgid / {
    if (mode == 2) {
        emit(cur_id, buf)
    }
    mode = 1
    buf = ""
    if ($0 == "msgid \"\"") {
        next
    }
    line = $0
    sub(/^msgid "/, "", line)
    sub(/"$/, "", line)
    buf = line
    next
}

/^msgstr / {
    if (mode != 1) {
        next
    }
    cur_id = buf
    buf = ""
    mode = 2
    if ($0 == "msgstr \"\"") {
        next
    }
    line = $0
    sub(/^msgstr "/, "", line)
    sub(/"$/, "", line)
    buf = line
    next
}

/^"/ {
    if (mode != 1 && mode != 2) {
        next
    }
    line = $0
    sub(/^"/, "", line)
    sub(/"$/, "", line)
    buf = buf line
    next
}

/^$/ {
    if (mode == 2) {
        emit(cur_id, buf)
    }
    reset_entry()
}

END {
    if (mode == 2) {
        emit(cur_id, buf)
    }
    print("\n}")
}
