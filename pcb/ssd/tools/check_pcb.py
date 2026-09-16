#!/usr/bin/env python3
"""Read-only A2 pad/network/footprint identity check; use KiCad's Python."""
from pathlib import Path
import json
import xml.etree.ElementTree as ET
import pcbnew as pcb

root = Path(__file__).resolve().parents[1]
manifest = json.loads((root/'review/design_manifest.json').read_text())
tree = ET.parse(root/'review/ssd.net.xml')
pin_nets = {}
for net in tree.findall('./nets/net'):
    name = net.get('name')
    # KiCad's XML unescapes '/' in pin-generated names; PCB stores {slash}.
    if name.startswith('Net-('):
        name = name.replace('/', '{slash}')
    for pin in net.findall('node'):
        pin_nets[(pin.get('ref'), pin.get('pin'))] = name
board = pcb.LoadBoard(str(root/'ssd.kicad_pcb'))
footprints = {f.GetReference(): f for f in board.GetFootprints()}
errors = []
expected_refs = {p['ref'] for p in manifest['parts']}
if set(footprints) != expected_refs:
    errors.append({'missing': sorted(expected_refs-set(footprints)),
                   'extra': sorted(set(footprints)-expected_refs)})
for part in manifest['parts']:
    fp = footprints.get(part['ref'])
    if fp is None:
        continue
    expected_path = part['path']+'/'+part['uuid']
    # KiCad GUI saves paths without the root schematic UUID; both forms
    # resolve to the same sheet and symbol. Keep the full remaining path check.
    root_prefix = '/' + expected_path.strip('/').split('/')[0]
    valid_paths = (expected_path, expected_path.removeprefix(root_prefix))
    if fp.GetPath().AsString() not in valid_paths:
        errors.append([part['ref'], 'schematic UUID path mismatch'])
    if fp.GetFPIDAsString() != part['footprint']:
        errors.append([part['ref'], 'footprint identifier mismatch'])
    if fp.GetValue() != part['value']:
        errors.append([part['ref'], 'value mismatch'])
    missing = set(part['nets']) - {pad.GetNumber() for pad in fp.Pads()}
    if missing:
        errors.append([part['ref'], 'missing numbered pads', sorted(missing)])
    for pad in fp.Pads():
        expected = pin_nets.get((part['ref'], pad.GetNumber()), '')
        if pad.GetNetname() != expected:
            errors.append([part['ref'], pad.GetNumber(), pad.GetNetname(), expected])
items = list(footprints.values())
for i, left in enumerate(items):
    for right in items[i+1:]:
        if left.GetBoundingBox(False,False).Intersects(right.GetBoundingBox(False,False)):
            errors.append(['footprint overlap', left.GetReference(), right.GetReference()])
report = {'pass': not errors, 'footprints': len(items),
          'pads': sum(len(list(f.Pads())) for f in items),
          'tracks': len(list(board.GetTracks())), 'errors': errors,
          'scope': 'Pad nets, footprint identities, values, schematic UUIDs and staged geometry. Manufacturing rules are checked separately by KiCad DRC.'}
print(json.dumps(report, indent=2))
raise SystemExit(1 if errors else 0)
