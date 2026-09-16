#!/usr/bin/env python3
"""Check the A2 full netlist against its reviewed pin-to-net manifest.

First export with KiCad 10: kicad-cli sch export netlist --format kicadxml
  -o review/ssd.net.xml ssd.kicad_sch
This checker never edits the schematic.
"""
from collections import defaultdict
from pathlib import Path
import json
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
manifest = json.loads((ROOT / 'review/design_manifest.json').read_text())
tree = ET.parse(ROOT / 'review/ssd.net.xml')
expected = defaultdict(set)
actual = defaultdict(set)
parts = {p['ref']: p for p in manifest['parts']}
errors = []
for part in parts.values():
    for pin, net in part['nets'].items():
        if net is not None:
            expected[net].add((part['ref'], pin))
for net in tree.findall('./nets/net'):
    for node in net.findall('node'):
        ref, pin = node.get('ref'), node.get('pin')
        if ref in parts and pin in parts[ref]['nets']:
            actual[net.get('name')].add((ref, pin))
expected_classes = {frozenset(nodes): net for net, nodes in expected.items()}
actual_classes = {frozenset(nodes): net for net, nodes in actual.items()}
for nodes, net in expected_classes.items():
    if nodes not in actual_classes:
        found = {name: sorted(pins) for name, pins in actual.items() if nodes & pins}
        errors.append({'expected_net': net, 'expected_pins': sorted(nodes), 'actual': found})
components = {c.get('ref'): c for c in tree.findall('./components/comp')}
if components.keys() != parts.keys():
    errors.append({'missing_parts': sorted(parts.keys() - components.keys()),
                   'extra_parts': sorted(components.keys() - parts.keys())})
for ref, part in parts.items():
    comp = components.get(ref)
    if comp is None:
        continue
    fp = comp.findtext('footprint')
    if fp != part['footprint']:
        errors.append({'ref': ref, 'footprint_mismatch': fp})
    if ref[0] in 'RC' and '0603_1608Metric' not in fp:
        errors.append({'ref': ref, 'not_0603': fp})
for power in ['+3V3', '+5V', 'GND']:
    if set(actual.get(power, ())) != expected[power]:
        errors.append({'power_net_mismatch': power})
report = {'pass': not errors, 'parts': len(parts), 'connected_pins': sum(map(len, expected.values())),
          'independent_nets': len(expected), 'resistors_0603': sum(r.startswith('R') for r in parts),
          'capacitors_0603': sum(r.startswith('C') for r in parts), 'errors': errors}
print(json.dumps(report, indent=2, ensure_ascii=False))
raise SystemExit(1 if errors else 0)
