"""Behavior checks for the catalog. Pass the compiled executable as the only argument."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

if len(sys.argv) != 2:
    raise SystemExit("Usage: python tests/test_catalog.py <catalog executable>")
EXECUTABLE = pathlib.Path(sys.argv.pop()).resolve()
SAMPLE = pathlib.Path(__file__).resolve().parents[1] / "data" / "courses.csv"


class CatalogTests(unittest.TestCase):
    def run_catalog(self, commands, files=None):
        with tempfile.TemporaryDirectory() as directory:
            for name, contents in (files or {}).items():
                pathlib.Path(directory, name).write_bytes(contents.encode("utf-8"))
            result = subprocess.run(
                [str(EXECUTABLE)], input=commands, text=True,
                cwd=directory, capture_output=True, timeout=3,
            )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, "")
        return result.stdout

    def test_sample_is_sorted_and_search_resolves_prerequisites(self):
        output = self.run_catalog(f"1\n{SAMPLE}\n2\n3\ncs410\n9\n")
        expected = (
            "CS120: Programming Foundations\nCS180: Discrete Structures\n"
            "CS240: Data Structures\nCS260: Computer Systems\n"
            "CS350: Database Applications\nCS410: Software Project\n"
        )
        self.assertIn(expected, output)
        self.assertIn("Prerequisites:\nCS350: Database Applications\nCS260: Computer Systems", output)

    def test_crlf_whitespace_case_and_optional_extension(self):
        output = self.run_catalog("1\ncourses\n3\n cs200 \n9\n", {
            "courses.csv": " cs100 , Intro \r\n\r\ncs200, Advanced , cs100 \r\n"
        })
        self.assertIn("CS200: Advanced\nPrerequisites:\nCS100: Intro", output)

    def test_invalid_exit_input_does_not_exit(self):
        output = self.run_catalog("9x\n2\n9\n")
        self.assertIn("Invalid option.", output)
        self.assertIn("No courses loaded.", output)

    def test_invalid_menu_values_recover(self):
        output = self.run_catalog("hello\n999999999999999999999999\n0\n2\n9\n")
        self.assertEqual(output.count("Invalid option."), 3)
        self.assertIn("No courses loaded.", output)

    def test_eof_at_each_prompt_exits(self):
        for commands in ("", "1\n", f"1\n{SAMPLE}\n3\n"):
            with self.subTest(commands=commands):
                self.assertIn("Exiting program.", self.run_catalog(commands))

    def test_invalid_reloads_preserve_previous_catalog(self):
        cases = {
            "blank ID": ",Intro\n",
            "blank title": "CS100,   \n",
            "missing title": "CS100\n",
            "duplicate ID": "CS100,Intro\ncs100,Duplicate\n",
            "missing prerequisite": "CS200,Advanced,CS999\n",
            "empty file": "",
            "quoted CSV": 'CS100,"Intro, Programming"\n',
        }
        for label, contents in cases.items():
            with self.subTest(case=label):
                output = self.run_catalog("1\ngood\n1\nbad\n2\n9\n", {
                    "good.csv": "CS120,Original Course\n", "bad.csv": contents,
                })
                self.assertIn("Course load failed. Any existing data kept.", output)
                self.assertIn("CS120: Original Course", output)
                self.assertNotIn("CS100: ", output)
                self.assertNotIn("CS200: ", output)

    def test_missing_file_preserves_previous_catalog(self):
        output = self.run_catalog("1\ngood\n1\nmissing\n2\n9\n", {
            "good.csv": "CS120,Original Course\n",
        })
        self.assertIn("Failed to open file", output)
        self.assertIn("CS120: Original Course", output)

    def test_successful_reload_replaces_old_catalog(self):
        output = self.run_catalog("1\nold\n1\nnew\n2\n9\n", {
            "old.csv": "CS100,Old Course\n", "new.csv": "CS200,New Course\n",
        })
        self.assertIn("CS200: New Course", output)
        self.assertNotIn("CS100: Old Course", output)

    def test_no_prerequisites_and_unknown_course(self):
        output = self.run_catalog("1\ncourses\n3\ncs100\n3\ncs999\n9\n", {
            "courses.csv": "CS100,Intro\n",
        })
        self.assertIn("CS100: Intro\nPrerequisites: None", output)
        self.assertIn("Course not found.", output)


if __name__ == "__main__":
    unittest.main()
