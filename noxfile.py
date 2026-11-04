import nox


@nox.session
def tests(session):
    session.install("pytest")
    session.install("-e", ".")
    session.run("pytest", "-vv", "--ignore=tools/lichess-bot")


@nox.session
def lint(session):
    session.install("ruff", "codespell")
    session.run("ruff", "check", ".")
    session.run("codespell", "sisyphus", "tests", "README.rst")
