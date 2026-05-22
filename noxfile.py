import nox

@nox.session
def tests(session):
    session.install("pytest")
    session.install("-e", ".")
    session.run("pytest", "-vv") 