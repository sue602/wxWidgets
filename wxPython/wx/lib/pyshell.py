
"""Renamer stub: provides a way to drop the wx prefix from wxPython objects."""

__cvsid__ = "$Id$"
__revision__ = "$Revision$"[11:-2]

from wx import _rename
from wxPython.lib import pyshell
_rename(globals(), pyshell.__dict__, modulename='lib.pyshell')
del pyshell
del _rename
