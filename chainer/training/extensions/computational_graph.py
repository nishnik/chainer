import os
import subprocess

from chainer import computational_graph
from chainer import configuration
from chainer.training import extension
from chainer.utils import argument
from chainer import variable


# 2 functions copied from ChainerRL. Ref:
# https://github.com/chainer/chainerrl/blob/v0.4.0/chainerrl/misc/draw_computational_graph.py  # NOQA
def is_return_code_zero(args):
    """Return true iff the given command's return code is zero.

    All the messages to stdout or stderr are suppressed.

    Args:
        args (list of str): Command to execute.

    """

    with open(os.devnull, 'wb') as FNULL:
        try:
            subprocess.check_call(args, stdout=FNULL, stderr=FNULL)
        except subprocess.CalledProcessError:
            # The given command returned an error
            return False
        except OSError:
            # The given command was not found
            return False
        return True


def is_graphviz_available():
    """Tell whether graphviz is available or not."""
    return is_return_code_zero(['dot', '-V'])


_var_style = {'shape': 'octagon', 'fillcolor': '#E0E0E0', 'style': 'filled'}
_func_style = {'shape': 'record', 'fillcolor': '#6495ED', 'style': 'filled'}


def dump_graph(root_name, filename=None,
               variable_style=None, function_style=None, **kwargs):
    """Returns a trainer extension to dump a computational graph.

    This extension dumps a computational graph. The graph is output in DOT
    language. If graphviz is available, this also renders and saves the image
    of computational graph.

    It only dumps a graph at the first invocation.

    .. note::
       As of v2.0.0, the computational graph is not kept by default. This
       extension changes this behavior until the first invocation. **It is
       strongly recommended to use it with the default trigger setting.**

       The detailed behavior of this extension since v2.0.0 is as follows.

       1. In its initializer, it turns on the
          ``chainer.config.keep_graph_on_report`` flag.
       2. At the first iteration, it dumps the graph using the graph held by
          the reported variable.
       3. After dumping the graph, it turns off the flag (if it was originally
          turned off) so that any variable reported afterward does not hold
          a computational graph.

       When the ``keep_graph_on_report`` flag is turned on, the computational
       graph created by the updater is kept during the invocation of
       extensions. It will cause an unnecessarily large memory consumption
       when an extension also uses a large amount of memory, e.g.
       :class:`~chainer.training.extensions.Evaluator`.

       With the default setting, the ``dump_graph`` extension is called at the
       first iteration. Since :class:`~chainer.training.extensions.Evaluator`
       is not called at the first iteration in most cases, it does not cause
       any memory problem.

    Args:
        root_name (str): Name of the root of the computational graph. The
            root variable is retrieved by this name from the observation
            dictionary of the trainer.
        filename (str): Output file name. It is recommended to use this
            argument though, instead of `filename` you can specify name of
            the output file by `out_name` for backward compatibility.
            If both `filename` and `out_name` are specified, `filename`
            is used.
        variable_style (dict): Dot node style for variables. Each variable is
            rendered by an octagon by default.
        function_style (dict): Dot node style for functions. Each function is
            rendered by a rectangular by default.

    .. seealso::
       See :func:`~chainer.computational_graph.build_computational_graph`
       for the ``variable_style`` and ``function_style`` arguments.

    """
    def trigger(trainer):
        return trainer.updater.iteration == 1

    out_name, = argument.parse_kwargs(
        kwargs, ('out_name', 'cg.dot')
    )
    if filename is None:
        filename = out_name

    if variable_style is None:
        variable_style = _var_style
    if function_style is None:
        function_style = _func_style

    original_flag = [None]

    def initializer(_):
        original_flag[0] = configuration.config.keep_graph_on_report
        configuration.config.keep_graph_on_report = True

    @extension.make_extension(trigger=trigger, initializer=initializer)
    def dump_graph(trainer):
        try:
            var = trainer.observation[root_name]
            if not isinstance(var, variable.Variable):
                raise TypeError('root value is not a Variable')
            cg = computational_graph.build_computational_graph(
                [var],
                variable_style=variable_style,
                function_style=function_style
            ).dump()

            file_path = os.path.join(trainer.out, filename)
            with open(file_path, 'w') as f:
                f.write(cg)

            if is_graphviz_available():
                image_path = os.path.join(
                    trainer.out, os.path.splitext(filename)[0] + ".png"
                )
                subprocess.check_call(
                    ['dot', '-Tpng', file_path, '-o', image_path])
        finally:
            configuration.config.keep_graph_on_report = original_flag[0]

    return dump_graph
