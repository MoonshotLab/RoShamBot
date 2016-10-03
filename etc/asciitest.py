import re

CHOICES = ['r', 'p', 's']

SPACER = " " * 20

ASCII_ART = {
    'r':"""
    _______
---'   ____)
      (_____)
      (_____)
      (____)
---.__(___)        """,
    'p':"""
    _______
---'   ____)____
          ______)
          _______)
         _______)
---.__________)    """,
    's':"""
    _______
---'   ____)____
          ______)
       __________)
      (____)
---.__(___)        """
}

def revascii(str):
    return '\n'.join([line[::-1] for line in re.split(r'(\s+)', str)])

def main():
    for play in CHOICES:
        print ASCII_ART[play]
        print revascii(ASCII_ART[play])

if __name__ == "__main__":
    main()
