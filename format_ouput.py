###############################################################################
# Parses the Siadex output into the format required for pandaPIparser validator
###############################################################################

import sys
import re
from collections import OrderedDict

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

def parse_plan(lines, tasklist):
    """ Produces the plan part in the final output """
    output = []
    output.append("==>")

    used_ids = []    # List of already included ids

    for action in lines:
        line = action.replace(":action ", "")
        line = re.sub('_primitive', '', line)
        identifier_list = get_subtasks_ids([line], tasklist, used_ids, plan=True)
        identifier = identifier_list[0]
        
        used_ids.append(identifier)

        # Ignore goal_action
        if not "goal_action" in line[1:-1]:
            output.append(str(identifier) + " " + line[1:-1])

    return output

# ------------------------------------------------------------------------------

def get_DT(output_part):
    """ Produces the decomposition part in the final output """
    output_part = output_part.split("Lista de tareas: \n")

    # Iterate for each tasks, getting the important info from its method
    info, roots, unif = get_method_info(output_part[0])

    # Get tasks and their ids
    tasks_headers, tasks_ids = get_tasks(output_part[1], unif)

    # Remove goal_action task
    roots = remove_goal_action_from_root(roots, tasks_ids)

    # Construct the decomposition tree    
    return parse_DT(tasks_headers, info, roots, tasks_ids), tasks_headers

# ------------------------------------------------------------------------------

def remove_goal_action_from_root(roots, tasks_ids):
    roots_copy = list(roots)

    for r in roots:
        if "goal_action" in tasks_ids[r]:
            roots_copy.remove(r)

    return roots_copy

# ------------------------------------------------------------------------------

# Doesn't store the primitives
def get_tasks(tasks, unif):
    """ Returns two dictionaries: id-task_header and id-task_name """
    lines = list(filter(None, tasks.splitlines()))
    tasks_headers = OrderedDict()
    tasks_names = OrderedDict()

    for line in lines:
        identifier =  re.search(r"\[([0-9_]+)\]", line).group(1)

        # Instantiate variables
        if "?" in line:            
            variables = re.findall(r"(\?.+?)[\ )]", line)
            
            for var in variables:
                if unif.get(var):
                    value = unif[var]
                    line = line.replace(var, value)

        header = re.search(r"(?::([^.]+)) (\(.*\))", line).group(2)
        # Name of the task
        match = re.search(r"(?::([^.]+)) \((.*?)\)\ ?", line)

        if match:
            name = match.group(2)
            tasks_headers.setdefault(identifier,header)
            tasks_names.setdefault(identifier,name)

    return tasks_headers, tasks_names

# ------------------------------------------------------------------------------

def get_method_info(text):
    """ Returns information for each tasks and the root ones """
    # Split by delimiter
    blocks = text.split("===")

    # Get roots ids
    roots = blocks[0].split("Root:")[1].split("-")
    roots.remove('\n')

    # Remove empty lines
    while '\n' in blocks:
        blocks.remove('\n')

    # Get unifications, if any
    unif_text = blocks[-1].split('\n')
    # Convert to dict
    unif = {}
    if not "Tarea" in blocks[-1]: # Make sure there are unifications
        for u in unif_text:
            if u:
                u = u.split(' <- ')
                unif[u[0]] = u[1]

    info = []
    for block in blocks[1:-1:]:
        if block != '\n':
            inf = parse_block(block)
            info.append(inf)

    return info, roots, unif

# ------------------------------------------------------------------------------

def parse_block(block):
    """ Returns the task id, the name of the method and of its subtasks """
    # Task id
    task =  re.search(r"Tarea:(\d+)", block).group(1)

    # Method name (ignores wrapper methods)
    method = None
    match = re.search(r":method (\w+(?:-\w+)*)", block)
    if match:
        method = match.group(1)

    # If it is not a primitive
    subtasks_names = None
    if method:
        subtasks = block.split(":tasks (")[1]   # Removing non necessary lines
        subtasks = re.findall(r"(\([\w -]+\))", subtasks)
        subtasks_names = []

        # Get name of the subtasks 
        for subt in subtasks:
            subtasks_names.append(subt.strip())
        
    return (task, method, subtasks_names)

# ------------------------------------------------------------------------------

def get_subtasks_ids(subtasks, taskslist, used_ids, plan=False):
    ids = []
    dic = {}  
    
    for subt in subtasks:
        for ident, name in taskslist.items():
            if name == subt:
                if ident not in used_ids: # If id not already used in another task
                    if not plan:    # If we are not looking ids for the plan part
                        if not dic.get(name, None):
                            ids.append(ident)
                            dic.setdefault(name, ident)                    
                    else:
                        ids.append(ident)

    return ids

# ------------------------------------------------------------------------------

def parse_DT(tasks, info, roots, tasks_names):
    output = []

    # Produce root line
    output.append("root " + ' '.join(str(x) for x in roots))

    used_ids = []    # List of already included ids

    for task in info:
        ident = task[0]
        method = task[1]

        if method != None:
            used_ids.extend(ident)  # Mark actual id as used
            subtasks = get_subtasks_ids(task[2], tasks, used_ids)
            used_ids.extend(subtasks) # Mark subtasks ids as used

            # len==0 -> An inline method (empty subtasks)
            # len==1 && !'primitive' -> not a wrapper
            if len(subtasks) > 1 or len(subtasks) == 0 or (len(subtasks) == 1 and 'primitive' not in tasks[subtasks[0]]):
                output.append(ident + " " + tasks[ident][1:-1] + " -> " + method 
                                    + " " + " ".join(subtasks))

    output.append("<==")

    return output

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

def main(argv):
    # --------------------------------------------------------------------------
    # Opening input plan

    with open(argv[1], "r") as f:    
        output = f.read()
        # Remove until 'Root' is seen    
        index = output.find("Root")
        output = output[index:]

        # Separating the plan and the decomposition
        output_parts = output.split("###")

    # If no output received from planner 
    if len(output_parts) < 2:
        print("[Error]: No output received from planner")
        sys.exit(1)

    # Removing empty lines    
    lines = list(filter(None, output_parts[1].splitlines()))

    decomposition, tasklist = get_DT(output_parts[0])
    plan = parse_plan(lines, tasklist)
    plan.extend(decomposition)

    # ----------------------------------------------------------------------
    # Printing the output

    for line in plan:
        print(line)
    

if __name__ == "__main__":
    main(sys.argv)