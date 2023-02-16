# import tkinter as tk
# import client

# def check_login():
#     username = username_entry.get()
#     password = password_entry.get()
#     if username == "admin" and password == "password":
#         login_label.config(text="Login successful")
#     else:
#         login_label.config(text="Incorrect username or password")

# # create the main window
# window = tk.Tk()
# window.geometry('800x500')
# window.title("Login Screen")

# # create the username label and entry widget
# username_label = tk.Label(window, text="Username:")
# username_label.pack()
# username_entry = tk.Entry(window)
# username_entry.pack()

# # create the password label and entry widget
# password_label = tk.Label(window, text="Password:")
# password_label.pack()
# password_entry = tk.Entry(window, show="*")
# password_entry.pack()

# # create the login button and label widget
# login_button = tk.Button(window, text="Login", command=check_login)
# login_button.pack()
# login_label = tk.Label(window, text="")
# login_label.pack()

# # run the main event loop
# window.mainloop()