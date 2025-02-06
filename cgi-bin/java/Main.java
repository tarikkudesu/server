import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.util.function.Consumer;
import java.lang.annotation.Target;
import java.nio.file.Paths;
import java.nio.file.Files;
import java.io.IOException;
import java.util.stream.*;
import java.util.*;

@View(post= "post.html", index = "index.html", form="form.html")
public class Main
{
	String response;
	View view;
	Consumer<String> consumer;
	Map<String, String> queryString;

	{
		consumer = this::loadFile;
		queryString = loadQueryString();
		view = this.getClass().getDeclaredAnnotation(View.class);
	}
	
	public Main()
	{
		try
		{
			if ("GET".equals(System.getenv("REQUEST_METHOD")))
			{
				if (!queryString.containsKey("action"))
					consumer.accept(System.getenv("user.dir") + view.index());
				else if ("form".equals(queryString.get("action")))
					consumer.accept(System.getenv("user.dir") + view.form());
				else
					consumer.accept(System.getenv("user.dir") + view.index());
				throw new Exception("");
			}
			else if ("POST".equals(System.getenv("REQUEST_METHOD")))
				consumer.accept(System.getenv("user.dir") + view.post());
		}
		catch (Exception e)
		{
			response = "<h1 style=\"text-align=center;\">action not exist</h1>";	
		}
		System.out.println(response);
	}

	public static void main(String [] args)
	{
		new Main();
	}
	
	void loadFile(String path)
	{
		try{
			response = new String(Files.readAllBytes(Paths.get(path)));
		}
		catch (Exception e)
		{
			response = "<h1 style=\"text-align=center;\">internal server error</h1>";	
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

}


	

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@interface View
{
	String post();
	String form();
	String index();
}
