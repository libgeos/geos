## Static Site Build

Our site is built with [hugo](https://gohugo.io/). To work with the docs you will probably want to [install hugo](https://gohugo.io/getting-started/installing/#quick-install) so you can see the rendered result as you work.

## Theme

We are using the [hugo-geekdoc](https://geekdocs.de/) theme for this web site.
To refresh the theme, update the version to the desired release and fetch the new code.

```bash
THEME_VERSION=v0.8.2
THEME=hugo-geekdoc
URL=https://github.com/thegeeklab/$THEME/releases/download/$THEME_VERSION/$THEME.tar.gz
rm -rf themes/$THEME
curl -sSL "$(URL)" | tar -xz -C themes/$THEME/ --strip-components=1
```

## Development

Install the hugo binary for your operating system. Run the local server.

```
hugo server
```

Remember to set "draft: false" in the page front matter or your content won't show up.
