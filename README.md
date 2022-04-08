# Siadex_Planner

Fdez-Olivares, J., Castillo, L., García-Pérez, Ó., and Palao, F. (2006). Bringing Users and Planning Technology Together. Experiences in SIADEX. pages 11–20.

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
