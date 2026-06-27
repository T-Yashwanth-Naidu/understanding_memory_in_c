#!/usr/bin/env python3
import os, json, datetime, anthropic

REPO  = "/home/yash/repo/understanding_memory_in_c"
LOG   = os.path.expanduser("~/.mem_c_chat.jsonl")
HIST  = os.path.expanduser("~/.mem_c_history.json")
MODEL = "claude-opus-4-8"

SYSTEM = ("You are a terse C memory tutor. Topics: stack vs heap lifetime, "
          "pointer and array decay, dangling pointers, double free, alignment, "
          "aliasing, strict aliasing, undefined behavior, valgrind and ASan "
          "findings. Show minimal compilable code. No filler, no praise. When I "
          "state memory semantics wrong, say so directly and show why with a "
          "concrete example or the relevant rule.")

def load_history():
    return json.load(open(HIST)) if os.path.exists(HIST) else []

def save_history(h):
    json.dump(h, open(HIST, "w"))

def log_turn(role, text):
    rec = {"ts": datetime.datetime.now().isoformat(), "role": role, "text": text}
    with open(LOG, "a") as f:
        f.write(json.dumps(rec) + "\n")

def main():
    client  = anthropic.Anthropic()
    history = load_history()
    print("mem-c chat. ctrl-d to exit.\n")
    while True:
        try:
            user = input("you> ").strip()
        except EOFError:
            print(); break
        if not user:
            continue
        history.append({"role": "user", "content": user})
        log_turn("user", user)

        text = ""
        with client.messages.stream(model=MODEL, max_tokens=2000,
                                     system=SYSTEM, messages=history) as stream:
            print("claude> ", end="", flush=True)
            for chunk in stream.text_stream:
                print(chunk, end="", flush=True)
                text += chunk
            print("\n")

        history.append({"role": "assistant", "content": text})
        log_turn("assistant", text)
        save_history(history)

if __name__ == "__main__":
    main()
