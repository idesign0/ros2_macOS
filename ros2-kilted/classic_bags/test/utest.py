import unittest
import pathlib
from classic_bags import Bag
from std_msgs.msg import Int32, String
import shutil
import tempfile


class BagTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(BagTest, self).__init__(*args, **kwargs)
        self.bag_dir = pathlib.Path(tempfile.mkdtemp(prefix='py_tmp_test_dir'))

    def tearDown(self):
        shutil.rmtree(self.bag_dir)

    def _write_simple_bag(self, name, storage_id):
        with Bag(name, 'w', format=storage_id) as bag:
            s = String(data='muppets')
            i = Int32(data=42)

            bag.write('chatter', s)
            bag.write('numbers', i)

    def check_value_equality(self, fname, storage_id):
        with Bag(fname, format=storage_id) as bag:
            numbers = list(bag.read_messages('numbers'))
            chatter = list(bag.read_messages('chatter'))

        self.assertEqual(len(numbers), 1)
        self.assertEqual(len(chatter), 1)

        numbers = numbers[0]
        chatter = chatter[0]

        # channel names
        self.assertEqual(numbers[0], 'numbers')
        self.assertEqual(chatter[0], 'chatter')

        # values
        self.assertEqual(numbers[1].data, 42)
        self.assertEqual(chatter[1].data, 'muppets')

    def check_type_equality(self, fname, storage_id):
        with Bag(fname, format=storage_id) as bag:
            numbers = next(bag.read_messages('numbers'))
            chatter = next(bag.read_messages('chatter'))

        self.assertEqual(numbers[1].__class__, Int32)
        self.assertEqual(chatter[1].__class__, String)

    def check_type_isinstance(self, fname, storage_id):
        with Bag(fname, format=storage_id) as bag:
            numbers = next(bag.read_messages('numbers'))
            chatter = next(bag.read_messages('chatter'))

        self.assertIsInstance(numbers[1], Int32)
        self.assertIsInstance(chatter[1], String)

    def test_expected_combos(self):
        for write_fmt, read_fmt in [('', ''),
                                    ('mcap', 'mcap'),
                                    ('sqlite3', 'sqlite3'),
                                    ('mcap', ''),
                                    ('sqlite3', ''),
                                    ]:
            name = f'bag_{write_fmt or "default"}_{read_fmt or "default"}'
            with self.subTest(name):
                bag_path = self.bag_dir / name
                self._write_simple_bag(bag_path, write_fmt)
                self.check_value_equality(bag_path, read_fmt)
                self.check_type_equality(bag_path, read_fmt)
                self.check_type_isinstance(bag_path, read_fmt)

    def test_expected_failures(self):
        for write_fmt, read_fmt in [('sqlite3', 'mcap'),
                                    ('mcap', 'sqlite3'),
                                    ]:
            name = f'bag_{write_fmt or "default"}_{read_fmt or "default"}'
            with self.subTest(name):
                bag_path = self.bag_dir / name
                self._write_simple_bag(bag_path, write_fmt)

                with self.assertRaises(RuntimeError):
                    bag = Bag(bag_path, format=read_fmt)
                    next(bag.read_messages())


if __name__ == '__main__':
    unittest.main()
