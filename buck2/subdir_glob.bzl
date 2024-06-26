# Copyright (c) 2017 LoopPerfect
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


"""Provides utility macros for working with globs."""

def _paths_join(path, *others):
    """Joins one or more path components."""
    result = path

    for p in others:
        if p.startswith("/"):  # absolute
            result = p
        elif not result or result.endswith("/"):
            result += p
        else:
            result += "/" + p

    return result

def subdir_glob(glob_specs, exclude = None, prefix = ""):
    """Returns a dict of sub-directory relative paths to full paths.

    The subdir_glob() function is useful for defining header maps for C/C++
    libraries which should be relative the given sub-directory.
    Given a list of tuples, the form of (relative-sub-directory, glob-pattern),
    it returns a dict of sub-directory relative paths to full paths.

    Please refer to native.glob() for explanations and examples of the pattern.

    Args:
      glob_specs: The array of tuples in form of
        (relative-sub-directory, glob-pattern inside relative-sub-directory).
        type: List[Tuple[str, str]]
      exclude: A list of patterns to identify files that should be removed
        from the set specified by the first argument. Defaults to [].
        type: Optional[List[str]]
      prefix: If is not None, prepends it to each key in the dictionary.
        Defaults to None.
        type: Optional[str]

    Returns:
      A dict of sub-directory relative paths to full paths.
    """
    if exclude == None:
        exclude = []

    results = []

    for dirpath, glob_pattern in glob_specs:
        results.append(
            _single_subdir_glob(dirpath, glob_pattern, exclude, prefix),
        )

    return _merge_maps(*results)

def _merge_maps(*file_maps):
    result = {}
    for file_map in file_maps:
        for key in file_map:
            if key in result and result[key] != file_map[key]:
                fail(
                    "Conflicting files in file search paths. " +
                    "\"%s\" maps to both \"%s\" and \"%s\"." %
                    (key, result[key], file_map[key]),
                )

            result[key] = file_map[key]

    return result

def _single_subdir_glob(dirpath, glob_pattern, exclude = None, prefix = None):
    if exclude == None:
        exclude = []
    results = {}
    files = native.glob([_paths_join(dirpath, glob_pattern)], exclude = exclude)
    for f in files:
        if dirpath:
            key = f[len(dirpath) + 1:]
        else:
            key = f
        if prefix:
            key = _paths_join(prefix, key)
        results[key] = f

    return results
