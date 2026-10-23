"""Run: python tests/protobuf/test_staticdata1533.py (requires grpcio-tools)."""
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

from google.protobuf.message import DecodeError
from grpc_tools import protoc

ROOT = Path(__file__).resolve().parents[2]


class StaticdataLayoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.output = tempfile.TemporaryDirectory()
        proto = ROOT / 'src' / 'protobuf' / 'staticdata.proto'
        result = protoc.main(['protoc', f'-I{proto.parent}', f'--python_out={cls.output.name}', str(proto)])
        if result:
            raise RuntimeError('Failed to compile staticdata.proto')
        generated = Path(cls.output.name) / 'staticdata_pb2.py'
        spec = importlib.util.spec_from_file_location('staticdata_pb2', generated)
        cls.pb = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.pb)

    @classmethod
    def tearDownClass(cls):
        cls.output.cleanup()

    def test_legacy_layout_preserved(self):
        original = self.pb.Staticdata()
        original.monsters.add(raceId=2, name='orc warlord')
        original.bosses.add(raceId=46, name='Black Knight')
        original.achievements.add(id=2, name='Chorister')
        original.houses.add(id=101, name='Targuna Cottage 1', description='Old house')
        original.quests.add(id=3, name='The Paradox Tower')
        decoded = self.pb.Staticdata.FromString(original.SerializeToString())
        self.assertEqual(decoded.bosses[0].name, 'Black Knight')
        self.assertEqual(decoded.houses[0].id, 101)

    def test_modern_layout_separates_houses_from_creatures(self):
        original = self.pb.Staticdata1533()
        original.monsters.add(raceId=2, name='orc warlord')
        original.creature_categories.add(id=1, name='Amphibic')
        original.achievements.add(id=2, name='Chorister')
        # Field 3 is text in House but a nested Outfit in the legacy Creature.
        original.houses.add(id=101, name='Targuna Cottage 1', description='Old house')
        original.bosses.add(raceId=46, name='Black Knight')
        original.quests.add(id=3, name='The Paradox Tower')
        wire = original.SerializeToString()
        with self.assertRaises(DecodeError):
            self.pb.Staticdata.FromString(wire)
        decoded = self.pb.Staticdata1533.FromString(wire)
        self.assertEqual(decoded.bosses[0].raceId, 46)
        self.assertEqual(decoded.creature_categories[0].name, 'Amphibic')
        self.assertEqual(decoded.quests[0].id, 3)

    def test_installed_1533_assets(self):
        assets = ROOT / 'data' / 'things' / '1533'
        catalog = assets / 'catalog-content.json'
        if not catalog.exists():
            self.skipTest('Optional Qt 1533 assets are not installed')
        entry = next(e for e in json.loads(catalog.read_text()) if e['type'] == 'staticdata')
        wire = (assets / entry['file']).read_bytes()
        decoded = self.pb.Staticdata1533.FromString(wire)
        for bank in ('monsters', 'bosses', 'creature_categories', 'houses', 'achievements', 'quests'):
            self.assertGreater(len(getattr(decoded, bank)), 0, bank)
        print('Parsed installed 1533 staticdata:', {bank: len(getattr(decoded, bank)) for bank in
              ('monsters', 'creature_categories', 'achievements', 'houses', 'bosses', 'quests')})


if __name__ == '__main__':
    unittest.main()
