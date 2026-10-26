"""Writes tes3mp-schema.json for the flight analyzer: the name of every TES3MP packet ID, read from
components/openmw-mp/NetworkMessages.hpp, so that `flight.py print/align --schema tes3mp-schema.json` shows
ID_PLAYER_POSITION instead of msg42.

Usage: python -I tools/coop-anything/make_tes3mp_schema.py <path to CrabNet MessageIdentifiers.h>
Run it again after adding a packet.
"""
import json
import pathlib
import re
import sys

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent.parent


def enum_body(text, name):
    match = re.search(r"enum\s+" + name + r"\s*\{(.*?)\};", text, re.S)
    if not match:
        sys.exit("enum %s not found" % name)
    body = re.sub(r"//[^\n]*|/\*.*?\*/", "", match.group(1), flags=re.S)
    return [entry.strip() for entry in body.split(",") if entry.strip()]


def main():
    identifiers = pathlib.Path(sys.argv[1]).read_text(errors="replace")
    raknet = {}
    value = -1
    for entry in enum_body(identifiers, "DefaultMessageIDTypes"):
        name, _, explicit = entry.partition("=")
        name = name.strip()
        if not re.fullmatch(r"[A-Za-z_]\w*", name):
            continue
        value = int(explicit.strip(), 0) if explicit.strip().isdigit() else value + 1
        raknet[name] = value

    messages = (ROOT / "components/openmw-mp/NetworkMessages.hpp").read_text()
    names = {}
    for name, number in raknet.items():
        if number < raknet["ID_USER_PACKET_ENUM"]:
            names[str(number)] = name
    value = None
    for entry in enum_body(messages, "GameMessages"):
        name, _, explicit = entry.partition("=")
        name = name.strip()
        if explicit:
            base, _, offset = explicit.partition("+")
            value = raknet[base.strip()] + (int(offset) if offset.strip() else 0)
        else:
            value += 1
        if value <= 255:
            names[str(value)] = name

    schema = {"game": "tes3mp", "message_types": names}
    out = HERE / "tes3mp-schema.json"
    out.write_text(json.dumps(schema, indent=1, sort_keys=True) + "\n")
    print("wrote %s with %d message names" % (out, len(names)))


if __name__ == "__main__":
    main()
