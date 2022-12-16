# HPDL-Planner

HPDL-Planner (also known as SIADEX) is a Hierarchical Task Network planner supporting partial order compound tasks, temporal and numeric planning.
You can read more about the planner [here](https://www.aaai.org/Papers/ICAPS/2006/ICAPS06-007.pdf).

The language used by the planner is HPDL, but you can also use the [pandaPIparser](https://github.com/panda-planner-dev/pandaPIparser) to transform from HDDL domains and problems.

## Installation

On the _planner_ directory, write:

```$ cmake . ``` 

```$ cmake --build . ``` 

And an executable called _planner_ will be produced.

## Requirements

- __makefile__
- __cmake__
- __flex__
- __bison__
- __g++__
- __python-dev__ (a restart will probably be required)
- __libreadline-dev__ (a restart will probably be required)

## Usage

Syntax: 

```$ ./planner [options] --domain_file (-d) <domain.hpdl> --problem_file (-p) <problem.hpdl>```

See: 

```$ ./planner --help``` 

for more information.

## Citation

If you would like to cite this planner in an scientific publication, please refer to this [paper](https://www.aaai.org/Papers/ICAPS/2006/ICAPS06-007.pdf):

```bibtex
@inproceedings{fdez2006bringing,
  title={Bringing users and planning technology together. Experiences in SIADEX},
  author={Fdez-Olivares, Juan and Castillo, Luis and Garc{\i}a-P{\'e}rez, Oscar and Palao, Francisco},
  year={2006}
}
```
