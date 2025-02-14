import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.util.function.Consumer;
import java.lang.annotation.Target;
import java.nio.file.Paths;
import java.nio.file.Files;
import java.util.stream.*;
import java.util.*;

@Render(post= "post.html", index = "index.html", form="form.html")
@JSP("cgi-bin/java/ressource/")
public class Main
{
	String response;
	Render render;
	JSP jsp;
	Consumer<String> consumer;
	Map<String, String> queryString;

	{
		consumer = this::loadFile;
		queryString = loadQueryString();
		jsp = this.getClass().getDeclaredAnnotation(JSP.class);
		render = this.getClass().getDeclaredAnnotation(Render.class);
	}

	public static void main(String [] args)
	{
		new Main();
	}

	public Main()
	{
		try
		{
			if ("GET".equals(System.getenv("REQUEST_METHOD")))
			{
				if (!queryString.containsKey("action"))
					consumer.accept(jsp.value() + render.index());
				else if ("form".equals(queryString.get("action")))
					consumer.accept(jsp.value() + render.form());
				else if ("post".equals(queryString.get("action")))
					throw new Exception("<h1 style=\"text-align:center;\">action not allowed on GET</h1>");
				else
					throw new Exception("<h1 style=\"text-align:center;\">action not exist</h1>");
			}
			else if ("POST".equals(System.getenv("REQUEST_METHOD")))
			{
				consumer.accept(jsp.value() + render.post());
				expand();
			}
			else
				throw new Exception("<h1 style=\\\"text-align:center;\\\">only GET and POST are allowed</h1>");
		}
		catch (Exception e)
		{
			response = e.getMessage();	
		}
		System.out.println(response);
	}
	
	void loadFile(String path)
	{
		try{
			response = new String(Files.readAllBytes(Paths.get(path)));
		}
		catch (Exception e)
		{
			response = "<h1 style=\"text-align:center;\">internal server error</h1>";	
		}
	}

	Map<String, String> loadQueryString()
	{
		String query = System.getenv("QUERY_STRING");
		if (query == null)
			return new HashMap<String, String>();
		return Arrays.stream(query.split("&"))
                .map(pair -> pair.split("="))
				.filter(pair -> pair.length == 2)
                .collect(Collectors.toMap(
					pair -> pair[0], pair -> pair[1],
					(existing, replacement) -> replacement));
	}


	void expand()
	{
		int i = 0;
		for(String str : queryString.values())
			response = response.replace("$" + ++i, str);
	}

}

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@interface Render
{
	String post();
	String form();
	String index();
}

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@interface JSP
{
	String value();
}
