def replace_commas_with_newlines(file_path):
    # Read the original file
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()
    
    # Replace commas with newlines
    updated_content = content.replace(',', '\n')
    
    # Write the updated content back to the file
    with open("Name_Time_VideoInjection_tester", 'w', encoding='utf-8') as file:
        file.write(updated_content)

# Specify the path to your file
file_path = 'Name_Time_VideoInjection'
replace_commas_with_newlines(file_path)
