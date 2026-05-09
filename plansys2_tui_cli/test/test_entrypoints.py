import unittest
from importlib.metadata import entry_points


class TestConsoleEntryPoints(unittest.TestCase):
    def test_plansys2_tui_entrypoint_loads(self):
        eps = entry_points(group="console_scripts", name="plansys2_tui")
        ep = next(iter(eps), None)

        self.assertIsNotNone(
            ep,
            "Console script 'plansys2_tui' is not registered in console_scripts.",
        )
        self.assertEqual(
            ep.value,
            "plansys2_tui_cli.tui.app:run_app",
            "The 'plansys2_tui' entry point points to an unexpected target.",
        )

        try:
            loaded = ep.load()
        except Exception as exc:
            self.fail(
                "Failed to load the 'plansys2_tui' entry point: "
                f"{type(exc).__name__}: {exc}"
            )

        self.assertTrue(
            callable(loaded),
            "The loaded 'plansys2_tui' entry point is not callable.",
        )


if __name__ == "__main__":
    unittest.main()
