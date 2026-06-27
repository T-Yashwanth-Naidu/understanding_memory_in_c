#!/usr/bin/env python3
import os, re, glob, json, subprocess, datetime, anthropic

REPO   = "/home/yash/repo/understanding_memory_in_c"
CHAT   = os.path.expanduser("~/.mem_c_chat.jsonl")
OFFSET = os.path.expanduser("~/.mem_c_wrapup_offset")
RAW    = os.path.expanduser("~/.mem_c_raw")
DRAFT  = os.path.join(REPO, "DRAFT_LEARNINGS.md")
MODEL  = "claude-sonnet-4-6"

ANSI = re.compile(r'\x1b\[[0-9;?]*[a-zA-Z]|\r')

def new_chat():
    start = int(open(OFFSET).read()) if os.path.exists(OFFSET) else 0
    out = []
    if os.path.exists(CHAT):
        with open(CHAT) as f:
            f.seek(start)
            for line in f:
                r = json.loads(line)
                out.append(f"[{r['ts']}] {r['role']}: {r['text']}")
            open(OFFSET, "w").write(str(f.tell()))
    return "\n".join(out)

def new_sessions():
    out = []
    for path in sorted(glob.glob(os.path.join(RAW, "session-*.log"))):
        raw = open(path, errors="ignore").read()
        clean = ANSI.sub("", raw)
        out.append(f"--- {os.path.basename(path)} ---\n{clean[:6000]}")
        os.rename(path, path + ".done")
    return "\n".join(out)

def diff():
    r = subprocess.run(["git","-C",REPO,"diff","HEAD","--","*.c","*.h"],
                       capture_output=True, text=True)
    return r.stdout[:4000]

def distill(chat, term, dff):
    c = anthropic.Anthropic()
    prompt = (
      "Below are a C-memory tutoring chat, a recorded terminal session, and the "
      "uncommitted git diff, all timestamped. Correlate them: tie what I did in "
      "the terminal to what was explained in chat.\n"
      "Extract only concrete, non-obvious learnings about memory in C: stack vs "
      "heap lifetime, pointer/array decay, dangling pointers, double free, "
      "alignment, aliasing, undefined behavior, valgrind/ASan findings. Terse "
      "markdown bullets. Weight chat heaviest, then terminal crash/sanitizer "
      "output, then diff. Omit textbook filler. If nothing new, output exactly: "
      "(nothing new)\n\n"
      f"=== CHAT ===\n{chat}\n\n=== TERMINAL ===\n{term}\n\n=== DIFF ===\n{dff}\n")
    m = c.messages.create(model=MODEL, max_tokens=1200,
                          messages=[{"role":"user","content":prompt}])
    return "".join(b.text for b in m.content if b.type=="text").strip()

def main():
    chat, term, dff = new_chat(), new_sessions(), diff()
    if not any(x.strip() for x in (chat, term, dff)):
        print("nothing new."); return
    notes = distill(chat, term, dff)
    if notes == "(nothing new)":
        print("model found nothing worth recording."); return
    stamp = datetime.date.today().isoformat()
    with open(DRAFT, "a") as f:
        f.write(f"\n## {stamp} (DRAFT - edit into LEARNINGS.md, then delete)\n{notes}\n")
    print(f"draft -> {DRAFT}")
    subprocess.run([os.environ.get("EDITOR","vim"), DRAFT])

if __name__ == "__main__":
    main()
