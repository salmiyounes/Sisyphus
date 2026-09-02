import subprocess

from setuptools import setup
from setuptools.command.build_py import build_py


class CustomBuildPyCommand(build_py):
    def run(self):
        subprocess.check_call(
            ["make", "DEBUG=0", "DEBUG_DISABLE_PRINT=1", "DISABLE_ASSERT=1"],
            cwd="sisyphus",
        )
        super().run()


setup(
    name="sisyphus",
    version="0.1.0",
    author="Salmi Younes",
    description="Sisyphus: a Python Chess Engine/Library based on C backend",
    long_description=open("README.rst").read(),
    url="https://github.com/salmiyounes/Sisyphus",
    packages=["sisyphus"],
    package_dir={"sisyphus": "sisyphus"},
    package_data={"sisyphus": ["libchess.so"]},
    cmdclass={"build_py": CustomBuildPyCommand},
)
