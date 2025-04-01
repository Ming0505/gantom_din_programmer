# concatenate_source.py
import os
import sys # Import sys for stderr
import argparse # Import argparse for command-line arguments

# --- Argument Parsing ---
parser = argparse.ArgumentParser(description='Concatenate source files from the src directory with a tree view and line counts.')
parser.add_argument('-o', '--output', default='context.txt', 
                    help='Path to the output file (default: context.txt)')
args = parser.parse_args()

# Define the directory to scan
source_dir = 'src'
output_file_path = args.output # Get output file path from arguments

# Define the output string, list for included files, and dict for line counts
output_content = ""
included_files = []
file_line_counts = {} # To store line counts for each file
total_lines = 0 # To store the total lines concatenated

print(f"Starting scan in directory: {source_dir}", file=sys.stderr)

# Check if the source directory exists
if not os.path.isdir(source_dir):
    print(f"Error: Directory not found: {source_dir}", file=sys.stderr)
    sys.exit(1) # Exit if source directory doesn't exist

# --- Step 1: Collect all file paths ---
# Recursively walk through the directory tree
for root, dirs, files in os.walk(source_dir):
    # Process each file found
    for file in files:
        # Construct the full file path
        filepath = os.path.join(root, file)
        print(f"Found file: {filepath}", file=sys.stderr)
        included_files.append(filepath)

# Sort files for consistent order
included_files.sort()

# --- Step 1.5: Pre-calculate Line Counts --- 
print("Calculating line counts...", file=sys.stderr)
for filepath in included_files:
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            count = len(lines)
            file_line_counts[filepath] = count
            print(f"  {filepath}: {count} lines", file=sys.stderr)
    except Exception as e:
        print(f"  Error counting lines in {filepath}: {e}", file=sys.stderr)
        file_line_counts[filepath] = 0 # Store 0 if error

# --- Step 2: Generate Tree Visualization with Line Counts ---
tree_output = "Included files tree:\n"
tree_dict = {}
if not included_files:
    tree_output += f"(No files found in {source_dir})\n"
else:
    # Group files by directory
    for filepath in included_files:
        dir_name = os.path.dirname(filepath)
        if dir_name not in tree_dict:
            tree_dict[dir_name] = []
        tree_dict[dir_name].append(os.path.basename(filepath))

    # Build the tree string
    for dir_name in sorted(tree_dict.keys()):
        tree_output += f"{dir_name}/\n"
        files_in_dir = sorted(tree_dict[dir_name])
        for i, file_name in enumerate(files_in_dir):
            prefix = "└── " if i == len(files_in_dir) - 1 else "├── "
            # Construct full path again to lookup line count
            current_filepath = os.path.join(dir_name, file_name)
            line_count = file_line_counts.get(current_filepath, 'N/A') # Get count
            tree_output += f"  {prefix}{file_name} ({line_count} lines)\n"

output_content += tree_output + "\n---\n\n" # Add tree and separator to output

# --- Step 3: Process files and add content ---
# Function to get file content with header and pre-calculated line count
def get_file_data(filepath, line_count):
    global total_lines # Allow modification of total_lines
    try:
        print(f"Processing file: {filepath}", file=sys.stderr) # Debug output
        # Open file just to read content (line count already known)
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        # Update total lines
        total_lines += line_count

        # Create header with file path and line count
        header = f"# {filepath} ({line_count} lines)\n"
        # Return formatted data
        return header + content + "\n"
    except Exception as e:
        # Print error message to stderr if file processing fails
        print(f"Error processing file {filepath}: {e}", file=sys.stderr)
        return f"# Error processing file {filepath}: {e}\n" # Include error in output

# Append data for each included file
for filepath in included_files:
    # Get the pre-calculated line count
    line_count = file_line_counts.get(filepath, 0)
    output_content += get_file_data(filepath, line_count)

# --- Step 4: Write the final concatenated output to file ---
try:
    with open(output_file_path, 'w', encoding='utf-8') as f:
        f.write(output_content)
    # Print success message and final summary to stderr
    print(f"\nSuccessfully wrote concatenated output to: {output_file_path}", file=sys.stderr)
    print(f"Processed {len(included_files)} files with a total of {total_lines} lines.", file=sys.stderr)
except Exception as e:
    print(f"Error writing output to file {output_file_path}: {e}", file=sys.stderr)
    sys.exit(1) # Exit if writing fails 