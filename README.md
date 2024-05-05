mainOpt required in components
on attaches a listener defined by unique component functions -- REQUIRES DECLARED VARIABLE FOR IDING
listener and compilation required in component modifier definition
require jQuery

# alchemylang
a unique language framework for designing static webpages with minimal clutter, written in Elixir
TODO: move all of this to docs and a tutorial page within said docs


# Command Line Interface
&emsp;Alchemy provides a CLI for auto generating code and project data (inspired by scaffolding in the Ruby on Rails framework)


# Filetypes
&emsp;Alchemy does not require filetypes, but encourages the use of **.alc**.
&emsp;To build a web page, a minimum of two files are required, which can be generated easiest through the CLI.

**Alchemy Build Files**
An alchemy build file contains the project structure and outline, along with the project's imports and plugins.

**Alchemy Page Layout File**
A page file which is applied globally across all the pages.

**Alchemy Page File**
Defines the structure of an individual page, the 'meta' and 'view' of an Alchemy project.

**Alchemy Component File**
Defines components that are included in the alchemy project.
TODO: component scripting should be in alchemy, instead of javascript

**Alchemy Plugin File**
TODO: this


# Components
&emsp;Alchemy provides a base of components which has a constantly expanding base library based on the standard HTML5 architecture.
<details>
    <summary>Standard library of components</summary>
    <br>
    <strong>Component Name</strong><br>
    Aliases: compname, compname2<br>
    <em>Elevator pitch</em><br>
    Unique Modifier Functions: href, etc<br><br>
    <strong>Text</strong><br>
    Aliases: text, p, paragraph<br>
    <em>Renders text on the screen</em><br>
    Unique Modifier Functions: underline, italicized, bold, font, size, color<br><br>
</details><br>
&emsp;The components in Alchemy can be created as a <em>static component</em>, which allows only short term modification of the component by using option functions without including a modifier function, causing the modification to apply to only the most recent component. Alternatively, the component can be created as a <em>dynamic component</em>, which allows for the modification at any point while the page is rendered.<br>
&emsp;Components can only be created in a <em>view</em> architecture block. To create a component, the following code may be of help:<br>
<code>&thinsp;create text // Creates a static text component<br>
&thinsp;create text: variableNameHere // Creates a dynamic text component modifiable in the future as variableNameHere</code><br>
&emsp;To modify a component, a <strong>modifier function</strong> can be used.<br>
<code>create text = Take me to Google!<br>
&thinsp;redirect_to https://google.com<br>
&thinsp;underline false</code><br>
&emsp;or:<br>
<code>create text: a = Take me to Google!<br>
&thinsp;a.redirect_to https://google.com<br>
&thinsp;a.underline false</code><br>
&emsp;Modifier functions are parsed by their arguments, for example <code>false</code> being the first argument for <code>a.underline false</code>. Some arguments may be longer than one word, but in general it should be expected to be one word.<br>
&emsp;A <strong>unique modifier function</strong> is a modifier function that is used by one component specifically (or by multiple, but with different workings each time). This can include something like <code>font</code>, which would throw a syntax error if used on something like an image.<br>
<details>
    <summary>Standard library of modifier functions (No uniques)</summary>
    <br>
    <strong>Modifier Function Name</strong><br>
    <em>Elevator pitch</em><br>
    Example<br><br>
    <strong>redirect_to</strong><br>
    <em>Redirects on click to a specified URL</em><br>
    redirect_to https://google.com<br><br>
</details>


# Errors
&emsp;A **syntax error** occurs when a user's code does not follow the syntactical rules of Alchemy, such as typing ```creat text``` (instead of ```create text```), which would cause the compiler to respond with:
```build failed -> syntax error on main.alc:1 -> "creat text" -> symbol not found```
&emsp;A **origin error** occurs when a user runs a piece of code that is missing its predecessor. This can include [ TODO: ADD EXAMPLE HERE, original idea has basis on having a redirection tag and disabling underline, but underline would be available without that so the example is invalid. ]


# Warnings
warnings for:
components in same file as meta or views, vice versa
useless declaration (setting a variable name for something that doesnt really need it)
useless modifier (ex: underline false after underline true)
unused variables